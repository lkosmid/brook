#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "glruntime.hpp"

using namespace brook;

GLKernel::GLKernel(GLRunTime * runtime,
                   const void *sourcelist[]) :
   runtime((GLRunTime *) runtime)
{
   unsigned int i,j;
   const char**sources = NULL;
   const char *src;
   char *progcopy;
   char *c;

   assert (sourcelist);

   for (i=0; sourcelist[i] != NULL; i+=2) {

      const char* nameString = (const char*) sourcelist[i];
      assert (nameString);

      sources = (const char**)sourcelist[i+1];

      if (strncmp(nameString, "fp30", strlen("fp30")) == 0) break;
      if (strncmp(nameString, "arb", strlen("arb")) == 0) break;
   }

   if (sourcelist[i] == NULL ||
       sources == NULL || sources[0]==NULL) {
      fprintf (stderr, "GL: No kernel source found\n");
      exit(1);
   }

   // count the number of passes.
   for (npasses=0; sources[npasses]; npasses++)
     ;

   pass_id = (GLuint *) malloc (npasses * sizeof(GLuint));
   pass_out = (unsigned int *) malloc (npasses * sizeof(int));
   outstreams = (GLStream **) malloc (npasses * sizeof(GLStream *));

   // Allocate ids
   glGenProgramsARB(npasses, pass_id);
   CHECK_GL();

   // look for our annotations...
   int sourceIndex = 0; // TIM: evil hack
   for (j=0; j<npasses; j++) {
     src = sources[sourceIndex++];
     std::string s = src;

     if (!j) {
       s = s.substr( s.find("!!BRCC") );

       // next is the narg line
       s = s.substr( s.find("\n")+1 );
       s = s.substr( s.find(":")+1 );

       std::string argumentCountString = s.substr( 0, s.find("\n") );
       unsigned int argumentCount = (unsigned int)
         atoi( argumentCountString.c_str() );
       argumentUsesIndexof = (bool *) malloc (sizeof(bool) * argumentCount);

       for( i = 0; i < argumentCount; i++ ) {
         s = s.substr( s.find("\n")+1 );
         s = s.substr( s.find("##")+2 );
         // char typeCode = s[0];
         char indexofHint = s[1];
         argumentUsesIndexof[i] = (indexofHint == 'i');
       }

       /* Get the constant names */
       constnames = (char **)  malloc (sizeof (char *) * GL_MAX_CONSTS);

       for (i=0; i<GL_MAX_CONSTS; i++)
         constnames[i] = NULL;

       progcopy = strdup (src);
       c = progcopy;
       while (*c && (c = strstr (c, "#semantic main.")) != NULL) {
         char *name;
         char *constregstring;
         unsigned int constreg;

         c += strlen("#semantic main.");

         /* set name to the ident */
         name = c;

         do c++; while (*c != ' ');
         *c = '\0';

         do c++; while (*c != ':');
         do c++; while (*c == ' ');

         /* If we have not found a constant register,
         ** simply continue */
         if (*c != 'C')
           continue;

         c++;
         constregstring = c;
         while (*c >= '0' && *c <= '9') c++;
         *c = '\0';
         c++;

         constreg = atoi (constregstring);

         if (constreg > GL_MAX_CONSTS) {
           fprintf (stderr, "GL: Too many constant registers\n");
           exit(1);
         }

         constnames[constreg] = strdup (name);
       }
       free(progcopy);

       for (i=0; i<GL_MAX_CONSTS; i++) {
         if (constnames[i] == NULL) {
           constnames[i] = strdup ("__UNKNOWN_CONST#######");
           sprintf(constnames[i], "__UNKNOWN_CONST%d", i);
         }
       }
     }

     /* Get the output info */
     progcopy = strdup (src);
     c = strstr(progcopy, "!!multipleOutputInfo:");
     c += strlen("!!multipleOutputInfo:");
     *(strstr(c,":")) = '\0';
     pass_out[j] = atoi(c);
     c += strlen(c)+1;
     *(strstr(c,":")) = '\0';
     if (atoi(c) != 1) {
       fprintf (stderr, "GL:  GL backend does not support "
                "shaders with multiple outputs\n");
       exit(1);
     }
     free(progcopy);

     /* TIM: try to parse reduction stuff */
     progcopy = strdup (src);
     c = strstr(progcopy, "!!reductionFactor:");
     c += strlen("!!reductionFactor:");
     *(strstr(c,":")) = '\0';
     int factor = atoi(c);
     free(progcopy);
     if( factor != 0 && factor != 2 )
     {
       /* TIM: remove this pass from consideration... */
       npasses--;
       j--;
	   continue;
     }

     /* Load the program code */
     CHECK_GL();

     glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[j]);
     glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        strlen(src), (GLubyte *) src);

     /* Check for program errors */
     if (glGetError() == GL_INVALID_OPERATION) {
       GLint pos;
       int i;
       int line, linestart;

       progcopy = strdup (src);
       glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &pos);

       line = 1;
       linestart = 0;
       for (i=0; i<pos; i++) {
         if (progcopy[i] == '\n') {
           line++;
           linestart = i+1;
         }
       }
       fprintf ( stderr, "GL: Program Error on line %d\n", line);
       for (i=linestart; progcopy[i] != '\0' && progcopy[i] != '\n'; i++);
       progcopy[i] = '\0';
       fprintf ( stderr, "%s\n", progcopy+linestart);
       for (i=linestart; i<pos; i++)
         fprintf ( stderr, " ");
       for (;progcopy[i] != '\0' && progcopy[i] != '\n'; i++)
         fprintf ( stderr, "^");
       fprintf ( stderr, "\n");
       free(progcopy);
       fprintf ( stderr, "%s\n",
                 glGetString(GL_PROGRAM_ERROR_STRING_ARB));
       exit(1);
     }
   }

   for (i=0; i<5; i++)
      tmpReduceStream[i] = NULL;

   /* Initialize state machine */
   ResetStateMachine();
}

void GLKernel::PushStream(Stream *s) {

   GLStream *glstream = (GLStream *) s;

   inputReduceStream = glstream;
   inputReduceStreamTreg = treg;
   inputReduceStreamSreg = sreg;

   if (sreg==0)
     sreg0 = glstream;

   for (int i=0; i<glstream->getFieldCount(); i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
      glBindTexture(GL_TEXTURE_RECTANGLE_EXT, glstream->id[i]);
      CHECK_GL();
      sreg++;
   }

   sargs[treg] = glstream;

   if (argumentUsesIndexof[argcount])
     creg++;

   treg++;
   argcount++;

}

void GLKernel::PushIter(Iter *s) {

   GLIter *gliter = (GLIter *) s;

   iargs[treg] = gliter;
   treg++;
   argcount++;
}

void GLKernel::PushConstant(const float &val) {
#ifdef DEBUG
   std::cerr << "GL: Binding creg " << creg << " to float constant "
             << constnames[creg] << "\n";
#endif

   for (unsigned int i=0; i<npasses; i++) {
#if 1
      glProgramNamedParameter4fNV(pass_id[i], strlen(constnames[creg]),
                                 (const GLubyte *) constnames[creg],
                                 val, 0.0f, 0.0f, 0.0f);
#else
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg,
                                   val, 0.0f, 0.0f, 0.0f);
#endif
   }
   creg++;
   argcount++;
   CHECK_GL();
}

void GLKernel::PushConstant(const float2 &val) {
#ifdef DEBUG
   std::cerr << "GL: Binding creg " << creg << " to float2 constant "
             << constnames[creg] << "\n";
#endif

   for (unsigned int i=0; i<npasses; i++) {
#if 1
      glProgramNamedParameter4fNV(pass_id[i], strlen(constnames[creg]),
                                 (const GLubyte *) constnames[creg],
                                 val.x, val.y, 0.0f, 0.0f);
#else
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg,
                                   val.x, val.y, 0.0f, 0.0f);
#endif
   }
   creg++;
   argcount++;
   CHECK_GL();
}

void GLKernel::PushConstant(const float3 &val) {
#ifdef DEBUG
   std::cerr << "GL: Binding creg " << creg << " to float3 constant "
             << constnames[creg] << "\n";
#endif

   for (unsigned int i=0; i<npasses; i++) {
#if 1
      glProgramNamedParameter4fNV(pass_id[i], strlen(constnames[creg]),
                                 (const GLubyte *) constnames[creg],
                                 val.x, val.y, val.z, 0.0f);
#else
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg,
                                   val.x, val.y, val.z, 0.0f);
#endif
   }
   creg++;
   argcount++;
   CHECK_GL();
}

void GLKernel::PushConstant(const float4 &val) {
#ifdef DEBUG
   std::cerr << "GL: Binding creg " << creg << " to float4 constant "
             << constnames[creg] << "\n";
#endif

   for (unsigned int i=0; i<npasses; i++) {
#if 1
      glProgramNamedParameter4fNV(pass_id[i], strlen(constnames[creg]),
                                 (const GLubyte *) constnames[creg],
                                 val.x, val.y, val.z, val.w);
#else
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg,
                                   val.x, val.y, val.z, val.w);
#endif
   }
   creg++;
   argcount++;
   CHECK_GL();
}
void GLKernel::PushReduce(void * val, __BRTStreamType type) {

   reduceVal = val;
   reduceType = type;
   reduceSreg = sreg;
   reduceTreg = treg;

   if (sreg==0 && type == __BRTSTREAM)
     sreg0 =(GLStream *) (Stream *) *((::brook::stream*) val);

   sreg++;
   treg++;
   argcount++;
}

void GLKernel::PushGatherStream(Stream *s) {
   GLStream *glstream = (GLStream *) s;

   if (sreg==0)
     sreg0 = glstream;

   for (int i=0; i<glstream->getFieldCount(); i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
      glBindTexture(GL_TEXTURE_RECTANGLE_EXT, glstream->id[i]);
      CHECK_GL();

      sreg++;
   }

   creg++;
   argcount++;
}

void GLKernel::PushOutput(Stream *s) {
   GLStream *glstream = (GLStream *) s;

   if (argumentUsesIndexof[argcount]) {
     sargs[treg] = glstream;
     treg++;
     creg++;
   }

   outstreams[nout] = glstream;

   assert (nout < npasses);

   nout++;
   argcount++;
}


void GLKernel::ResetStateMachine() {
   /* Reset state machine */

   int i;

   nout = 0;
   creg = 0;
   sreg = 0;
   treg = 0;
   argcount = 0;
   for (i=0; i<GL_MAX_TEXCOORDS; i++) {
      sargs[i] = NULL;
      iargs[i] = NULL;
   }
   sreg0 = NULL;
}

#if 0
static void
dumpframebuffer (int w, int h) {
   int i, j;
   glfloat4 *pix = (glfloat4 *) malloc (w*h*sizeof(glfloat4));
   glReadPixels (0,0,w,h,GL_RGBA,GL_FLOAT,pix);

   fprintf (stderr, "---------\n");
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++)
         fprintf(stderr,"%3.2f ", pix[i*w+j].x);
      fprintf(stderr, "\n");
   }
   fprintf (stderr, "---------\n");
   fflush (stderr);
}

static void
clearframebuffer () {
   glClearColor(111.11f, 111.11f, 111.11f, 111.11f);
   glClear(GL_COLOR_BUFFER_BIT);
}
#else
#define dumpframebuffer(a,b)
#define clearframebuffer()
#endif

GLKernel::~GLKernel() {
   unsigned int i;

   free(pass_id);
   free(pass_out);

   for (i=0; i<GL_MAX_CONSTS; i++)
      free(constnames[i]);

   free(argumentUsesIndexof);

   for (i=0; i<5; i++)
      if (tmpReduceStream[i])
         delete (tmpReduceStream[i]);

}
