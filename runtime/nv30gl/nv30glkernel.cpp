
#include <stdio.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLKernel::NV30GLKernel(NV30GLRunTime * runtime,
                           const void *sourcelist[]) :
   runtime(runtime)
{
   unsigned int i, n;
   const char *source;
   char *progcopy;
   char *c;
   
   assert (sourcelist);
   
   for( i=0; sourcelist[i] != NULL; i+=2 ) {
      
      const char* nameString = (const char*) sourcelist[i];
      assert (nameString);
      
      source = (const char*) sourcelist[i+1];
      
      if( strncmp( nameString, 
                   "fp30", strlen("fp30"))  == 0 )
         break;
   }
   
   if (sourcelist[i] == NULL ||
       source == NULL) { 
      fprintf (stderr, "NV30GL: No kernel source found\n");
      exit(1);
   }
   
   /* Load the program code */
   CHECK_GL();
   
   glGenProgramsNV (1, &id);
   glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, id, strlen(source), 
                    (const GLubyte*) source);
   glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
   
   /* Check for program errors */
   if (glGetError() == GL_INVALID_OPERATION) {
      GLint pos;
      int i;
      int line, linestart;
      
      progcopy = strdup (source);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &pos);
      
      line = 1;
      linestart = 0;
      for (i=0; i<pos; i++) {
         if (progcopy[i] == '\n') {
            line++;
            linestart = i+1;
         }
      }
      fprintf ( stderr, "NV30GL: Program Error on line %d\n", line);
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
   
   /* Get the constant names */
   constnames = (char **)  malloc (sizeof (char *) * NV30GL_MAXCONSTS);
   
   for (i=0; i<NV30GL_MAXCONSTS; i++)
      constnames[i] = NULL;
   
   progcopy = strdup (source);
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
      
      if (constreg > NV30GL_MAXCONSTS) {
         fprintf (stderr, "NV30GL: Too many constant registers\n");
         exit(1);
      }
      
      constnames[constreg] = strdup (name);
   }
   free(progcopy);
   
   for (i=0; i<NV30GL_MAXCONSTS; i++) {
      if (constnames[i] == NULL) {
         constnames[i] = strdup ("__UNKNOWN_CONST#######");
         sprintf(constnames[i], "__UNKNOWN_CONST%d", i);
      }
   }
   
   /* Initialize GL State */
   glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, (GLint *) &n);
   for (i=0; i<n; i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+i);
      glEnable(GL_TEXTURE_RECTANGLE_NV);
   }
   CHECK_GL();

   /* Initialize state machine */
   creg = 1;
   sreg = 0;
   treg = 0;
   
   for (i=0; i<NV30GL_MAX_TEXCOORDS; i++)
      sargs[i] = NULL;
}

void NV30GLKernel::PushStream(Stream *s) {
     
   glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
   CHECK_GL();

   sargs[treg] = (NV30GLStream *) s;
   sreg++;
   treg++;
}

void NV30GLKernel::PushIter(Iter *s) {
   // XXX: TODO
   abort();
}

void NV30GLKernel::PushConstant(const float &val) {
   glProgramNamedParameter4fNV(id, strlen(constnames[creg]),
                               (const GLubyte *) constnames[creg],
                               val, 0.0f, 0.0f, 0.0f);
   creg++;
   CHECK_GL();
}

void NV30GLKernel::PushConstant(const float2 &val) {
   glProgramNamedParameter4fNV(id, strlen(constnames[creg]),
                               (const GLubyte *) constnames[creg],
                               val.x, val.y, 0.0f, 0.0f);
   creg++;
}

void NV30GLKernel::PushConstant(const float3 &val) {
   glProgramNamedParameter4fNV(id, strlen(constnames[creg]),
                               (const GLubyte *) constnames[creg],
                               val.x, val.y, val.z, 0.0f);
   creg++;
}

void NV30GLKernel::PushConstant(const float4 &val) {
   glProgramNamedParameter4fNV(id, strlen(constnames[creg]),
                               (const GLubyte *) constnames[creg],
                               val.x, val.y, val.z, val.w);
   creg++;
}
void NV30GLKernel::PushReduce(void * val, __BRTStreamType) {
   // XXX: TODO
   abort();
}

void NV30GLKernel::PushGatherStream(Stream *s) {
   glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
   CHECK_GL();
   
   sreg++;
   creg++;
}

void NV30GLKernel::PushOutput(Stream *s) {
   assert (outstream);
   outstream = (NV30GLStream *) s;
}

void NV30GLKernel::Map() {
   
   unsigned int i;
   int w, h;
   float fl, fb, fr, ft;
   float fs1[8], ft1[8], fs2[8], ft2[8];
   float fs_shift[8], ft_shift[8];

   if (outstream == NULL) {
      fprintf (stderr, "No output stream specified for kernel\n");
      exit(1);
   }
   
   w = outstream->width;
   h = outstream->height;

   fl = -1.0f;
   fb = -1.0f;
   fr = ((float)(w*2 - NV30GLRunTime::workspace)) / 
      ((float) NV30GLRunTime::workspace);
   ft = ((float)(h*2 - NV30GLRunTime::workspace)) / 
      ((float) NV30GLRunTime::workspace);

   /* Compute texture coordinates */
   for (i=0; i<8; i++) {
      NV30GLStream *s = sargs[i];
      if (s == NULL) continue;
      if (w == 1) {
         fs1[i] = s->width/2.0f;
         fs2[i] = fs1[i];
      } else {
         fs_shift[i] = ((float)s->width) / (2*w - 2);
         fs1[i] = -fs_shift[i];
         fs2[i] = s->width + fs_shift[i];
      }
   
      if (h == 1) {
         ft1[i] = s->height/2.0f;
         ft2[i] = ft1[i];
      } else {
         ft_shift[i] = ((float)s->height) / (2*h - 2);
         ft1[i] = -ft_shift[i];
         ft2[i] = s->height + ft_shift[i];
      }
   }

   /* Issue the poly */
   glBegin(GL_POLYGON);
   
   for (i=0; i<8; i++)
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB+1, fs1[i], ft1[i]);
   glVertex2f(fl, fb);
   
   for (i=0; i<8; i++)
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB+1, fs2[i], ft1[i]);
   glVertex2f(fr, fb);

   for (i=0; i<8; i++)
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB+1, fs2[i], ft2[i]);
   glVertex2f(fr, ft);

   for (i=0; i<8; i++)
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB+1, fs1[i], ft2[i]);
   glVertex2f(fl, ft);
   
   glEnd();

   glBindTexture (GL_TEXTURE_RECTANGLE_NV, outstream->id);
   glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                    GLtype[outstream->ncomp], 0, 0, w, h, 0);
   CHECK_GL();

   /* Reset state machine */
   creg = 1;
   sreg = 0;
   treg = 0;
   for (i=0; i<NV30GL_MAX_TEXCOORDS; i++)
      sargs[i] = NULL;
   outstream = NULL;
}

NV30GLKernel::~NV30GLKernel() {
   unsigned int i;
 
   for (i=0; i<NV30GL_MAXCONSTS; i++)
      free(constnames[i]);

}
