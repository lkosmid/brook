#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "glruntime.hpp"

using namespace brook;


static void
NV30ComputeTexCoords(unsigned int w, unsigned int h, 
                     bool is_iter, bool is_1D,
                     float x1, float y1, float z1, float w1,
                     float x2, float y2, float z2, float w2,
                     glfloat4 &f1, glfloat4 &f2);

static void
ATIComputeTexCoords(unsigned int w, unsigned int h, 
                    bool is_iter, bool is_1D,
                    float x1, float y1, float z1, float w1,
                    float x2, float y2, float z2, float w2,
                    glfloat4 &f1, glfloat4 &f2);

void
GLKernel::Initialize(GLRunTime * runtime, const void *sourcelist[],
                     const char *shaderLang)
{
   unsigned int i,j;
   const ::brook::desc::gpu_kernel_desc* kernelDesc = NULL;
   const char *src;
   char *progcopy;
   char *c;

   assert (sourcelist);

   for (i=0; sourcelist[i] != NULL; i+=2) {

      const char* nameString = (const char*) sourcelist[i];
      assert (nameString);

      kernelDesc = (const ::brook::desc::gpu_kernel_desc*)(sourcelist[i+1]);

      if (strncmp(nameString, shaderLang, strlen(shaderLang)) == 0) break;
   }

   if (sourcelist[i] == NULL ||
       kernelDesc == NULL || kernelDesc->_techniques.size()==0) {
      fprintf (stderr, "GL: No kernel source found\n");
      exit(1);
   }

   // TIM: the GL runtime does not currently support multiple techniques
   // we thus consider only the first:
   const ::brook::desc::gpu_technique_desc& techniqueDesc = kernelDesc->_techniques[0];
   npasses = (int)techniqueDesc._passes.size();

   pass_id = (GLuint *) malloc (npasses * sizeof(GLuint));
   pass_out = (unsigned int *) malloc (npasses * sizeof(int));
   outstreams = (GLStream **) malloc (npasses * sizeof(GLStream *));

   // Allocate ids
   glGenProgramsARB(npasses, pass_id);
   CHECK_GL();

   // look for our annotations...
   int sourceIndex = 0; // TIM: evil hack
   for (j=0; j<npasses; j++) {
     src = techniqueDesc._passes[j]._shaderString;
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
         fflush(stderr);
         assert(0);
         exit(1);
      }
   }

   switch (runtime->arch) {
   case ARCH_NV30:
      ComputeTexCoords = NV30ComputeTexCoords;
      break;
   case ARCH_ATI:
   default:
      ComputeTexCoords = ATIComputeTexCoords;
   }

   for (i=0; i<5; i++)
      tmpReduceStream[i] = NULL;

   /* Initialize state machine */
   ResetStateMachine();
}


GLKernel::~GLKernel() {
   unsigned int i;

   free(pass_id);
   free(pass_out);

   for (i=0; i<GL_MAX_CONSTS; i++)
      if (constnames[i])
         free(constnames[i]);
   free(constnames);

   free(argumentUsesIndexof);

   for (i=0; i<5; i++)
      if (tmpReduceStream[i])
         delete (tmpReduceStream[i]);

}

void
GLKernel::PushScaleBias(Stream *s)
{
}

void
GLKernel::PushStream(Stream *s) {

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
      PushShape(s);

   treg++;
   argcount++;
}


void
GLKernel::PushIter(Iter *s) {

   GLIter *gliter = (GLIter *) s;

   iargs[treg] = gliter;
   treg++;
   argcount++;
}


void
GLKernel::PushReduce(void * val, __BRTStreamType type) {

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


void
GLKernel::PushGatherStream(Stream *s) {
   GLStream *glstream = (GLStream *) s;

   if (sreg==0)
      sreg0 = glstream;

   for (int i=0; i<glstream->getFieldCount(); i++) {
      glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
      glBindTexture(GL_TEXTURE_RECTANGLE_EXT, glstream->id[i]);
      CHECK_GL();

      sreg++;
   }

   PushScaleBias(s);
   argcount++;
}


void
GLKernel::PushOutput(Stream *s) {
   GLStream *glstream = (GLStream *) s;

   if (argumentUsesIndexof[argcount]) {
      sargs[treg] = glstream;
      treg++;

      PushShape(s);
   }

   outstreams[nout] = glstream;

   assert (nout < npasses);

   nout++;
   argcount++;
}


void
GLKernel::ResetStateMachine() {
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


static void
NV30ComputeTexCoords(unsigned int w, unsigned int h, 
                     bool is_iter, bool is_1D,
                     float x1, float y1, float z1, float w1,
                     float x2, float y2, float z2, float w2,
                     glfloat4 &f1, glfloat4 &f2)
{
   const float half_pixel  = 0.5f / GLRunTime::workspace;
   const float pixel  = 1.0f / GLRunTime::workspace;

   if (is_iter && is_1D) {
      // 1D
      if (w == 1) {
         f1.x = x1; f1.y = y1;
         f1.z = z1; f1.w = w1;
         f2 = f1;
      } else {
         float sx = x2-x1;
         float sy = y2-y1;
         float sz = z2-z1;
         float sw = w2-w1;
         float ratiox = sx / w;
         float ratioy = sy / w;
         float ratioz = sz / w;
         float ratiow = sw / w;
         float shiftx = ratiox * 0.5f;
         float shifty = ratioy * 0.5f;
         float shiftz = ratioz * 0.5f;
         float shiftw = ratiow * 0.5f;
         f1.x = x1 - (shiftx - half_pixel);
         f1.y = y1 - (shifty - half_pixel);
         f1.z = z1 - (shiftz - half_pixel);
         f1.w = w1 - (shiftw - half_pixel);

         // Totally unclear why you need the + pixel???
         f2.x = (2*x2-x1) - (shiftx - half_pixel) + pixel;
         f2.y = (2*y2-y1) - (shifty - half_pixel) + pixel;
         f2.z = (2*z2-z1) - (shiftz - half_pixel) + pixel;
         f2.w = (2*w2-w1) - (shiftw - half_pixel) + pixel;
      }
   } else {
      // 2D
      if (w == 1) {
         f1.x = x1 + half_pixel;
         f2.x = f1.x;
      } else {
         float sw = x2-x1;
         float ratio = sw / w;
         float shift = ratio * 0.5f;
         f1.x = x1 - (shift - half_pixel);
         // Totally unclear why you need the + pixel???
         f2.x = (2*x2-x1) - (shift - half_pixel) + pixel;
      }

      if (h == 1) {
         f1.y = y1 + half_pixel;
         f2.y = f1.y;
      } else {
         /* XXXX
          ** It is totally irrational that I have to
          ** treat the y direction totally differently
          ** than the x direction.
          ** Through experimentation: ratio*2+1 works
          ** Is this an NVIDIA bug?
          **    -- Ian
          */
         float sh = y2-y1;
         float ratio = sh / h;
         float shift = ratio * 0.5f;
         f1.y = y1 - (shift - half_pixel * (ratio*2+1));
         f2.y = (2*y2-y1) - (shift - half_pixel * (ratio*2+1));
      }
      f1.z = 0.0f;
      f1.w = 1.0f;
      f2.z = 0.0f;
      f2.w = 1.0f;
   }
}

static void
ATIComputeTexCoords(unsigned int w, unsigned int h, 
                    bool is_iter, bool is_1D,
                    float x1, float y1, float z1, float w1,
                    float x2, float y2, float z2, float w2,
                    glfloat4 &f1, glfloat4 &f2)
{
   const float half_pixel  = 0.5f / GLRunTime::workspace;
   const float pixel  = 1.0f / GLRunTime::workspace;

   if (is_iter && is_1D) {
      // 1D
      if (w == 1) {
         f1.x = x1; f1.y = y1;
         f1.z = z1; f1.w = w1;
         f2 = f1;
      } else {
         float sx = x2-x1;
         float sy = y2-y1;
         float sz = z2-z1;
         float sw = w2-w1;
         float ratiox = sx / w;
         float ratioy = sy / w;
         float ratioz = sz / w;
         float ratiow = sw / w;
         float shiftx = ratiox * 0.5f;
         float shifty = ratioy * 0.5f;
         float shiftz = ratioz * 0.5f;
         float shiftw = ratiow * 0.5f;
         f1.x = x1 - (shiftx - half_pixel);
         f1.y = y1 - (shifty - half_pixel);
         f1.z = z1 - (shiftz - half_pixel);
         f1.w = w1 - (shiftw - half_pixel);

         f2.x = (2*x2-x1) - (shiftx - half_pixel);
         f2.y = (2*y2-y1) - (shifty - half_pixel);
         f2.z = (2*z2-z1) - (shiftz - half_pixel);
         f2.w = (2*w2-w1) - (shiftw - half_pixel);
      }
   } else {
      // 2D
      float bias = is_iter ? 0.0f : 0.5f;

      if (w == 1) {
         f1.x = x1 + half_pixel;
         f2.x = f1.x;
      } else {
         float sw = x2-x1;
         float ratio = sw / w;
         float shift = ratio * 0.5f;
         f1.x = x1 - (shift - half_pixel) + bias;
         f2.x = (2*x2-x1) - (shift - half_pixel) + bias;
      }

      if (h == 1) {
         f1.y = y1 + half_pixel;
         f2.y = f1.y;
      } else {
         float sh = y2-y1;
         float ratio = sh / h;
         float shift = ratio * 0.5f;
         f1.y = y1 - (shift - half_pixel) + bias;
         f2.y = (2*y2-y1) - (shift - half_pixel) + bias;
      }
      f1.z = 0.0f;
      f1.w = 1.0f;
      f2.z = 0.0f;
      f2.w = 1.0f;
   }
}


void
GLKernel::RecomputeTexCoords(unsigned int w, unsigned int h,
                             glfloat4 f1[], glfloat4 f2[])
{
   unsigned int i;

   for (i = 0; i< GL_MAX_TEXCOORDS; i++) {
      if (sargs[i]) {
         GLStream *s = sargs[i];
         ComputeTexCoords(w, h, false, false,
                          0.0f, 0.0f, 0.0f, 1.0f,
                          (float) s->width,
                          (float) s->height, 0.0f, 1.0f,
                          f1[i], f2[i]);
      } else if (iargs[i]) {
         GLIter *itr = (GLIter *) iargs[i];
         ComputeTexCoords(w, h, true, (itr->dims==1),
                          itr->min.x, itr->min.y, itr->min.z, itr->min.w,
                          itr->max.x, itr->max.y, itr->max.z, itr->max.w,
                          f1[i], f2[i]);
      }
   }
}

void
GLKernel::IssueMapTexCoords(glfloat4 f1[], glfloat4 f2[])
{
   unsigned int i;

   for (i=0; i<GL_MAX_TEXCOORDS; i++) {
      if (sargs[i] || (iargs[i] && iargs[i]->dims==2)) {
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
               f1[i].x, f2[i].y, 0.0f, 1.0f);
      } else if (iargs[i] && iargs[i]->dims==1) {
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
               f1[i].x, f1[i].y, f1[i].z, f1[i].w);
      }
   }
}


void
GLKernel::Map()
{
   unsigned int pass_idx, out_idx;
   unsigned int w = -1, h = -1;
   glfloat4 f1[GL_MAX_TEXCOORDS], f2[GL_MAX_TEXCOORDS];

   w = -1; h = -1;

   /*
    * Map() with single output has npasses equal to total of all fields in
    * all outputs (i.e. structures require one pass per field, not just one
    * pass).
    */
   for (pass_idx=0, out_idx = 0; pass_idx < npasses && out_idx < npasses; ) {
      if (w != outstreams[out_idx]->width || h != outstreams[out_idx]->height) {
         w = outstreams[out_idx]->width;
         h = outstreams[out_idx]->height;

         RecomputeTexCoords(w, h, f1, f2);
         glViewport(0, 0, w, h);
      }

      GLStream *curOutput = outstreams[out_idx];
      unsigned int nfields = curOutput->getFieldCount();
      for (unsigned int field_idx=0; field_idx < nfields; field_idx++) {
         /*
          * If we have a floatN pbuffer, but a floatM stream, recreate the
          * pbuffer.
          */

         runtime->createPBuffer(curOutput->ncomp[field_idx]);
         
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[pass_idx]);

         /*
          * We execute our kernel by using it to texture a triangle that
          * has vertices (-1, 3), (-1, -1), and (3, -1) which works out
          * nicely to contain the square (-1, -1), (-1, 1), (1, 1), (1, -1).
          */

         glBegin(GL_TRIANGLES);
         IssueMapTexCoords(f1, f2);
         glVertex2f(-1.0f, 3.0f);
         IssueMapTexCoords(f1, f1);
         glVertex2f(-1.0f, -1.0f);
         IssueMapTexCoords(f2, f1);
         glVertex2f(3.0f, -1.0f);
         glEnd();
         CHECK_GL();

         glActiveTextureARB(GL_TEXTURE0_ARB);
         glBindTexture(GL_TEXTURE_RECTANGLE_EXT, curOutput->id[field_idx]);
         glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, w, h);
         if (sreg0)
            glBindTexture(GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
         CHECK_GL();

         pass_idx++;
      }
      out_idx++;
   }

   ResetStateMachine();
}


static void
issue_reduce_poly (int x1, int y1,
      int w, int h, int n,
      glfloat4 f1[], glfloat4 f2[]) {

   int i;

   glViewport(x1,y1,w,h);

   /* Issue the poly */
   glBegin(GL_TRIANGLES);
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
            f1[i].x, f2[i].y, 0.0f, 1.0f);
   glVertex2f(-1.0f, 3.0f);
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
            f1[i].x, f1[i].y, 0.0f, 1.0f);
   glVertex2f(-1.0f, -1.0f);
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
            f2[i].x, f1[i].y, 0.0f, 1.0f);
   glVertex2f(3.0f, -1.0f);
   glEnd();
   CHECK_GL();
}


void
GLKernel::ReduceScalar()
{
   /* First reduce in the x direction */
   int i;
   unsigned int w = inputReduceStream->width;
   unsigned int h = inputReduceStream->height;
   int ncomp = inputReduceStream->ncomp[0];
   glfloat4 readback;
   GLStream *t;
   bool first = true;

   assert(npasses == 1);

   // Don't attempt reduce on 1x1
   if (w == 1 && h == 1) {
      inputReduceStream->Write(reduceVal);
      return;
   }

   // Initialize the pbuffer to the right dimension
   runtime->createPBuffer(ncomp);
   
   /* Construct the temp stream */
   if (tmpReduceStream[ncomp] &&
         (tmpReduceStream[ncomp]->width < (unsigned int) w ||
          tmpReduceStream[ncomp]->height < (unsigned int) h)) {
      delete tmpReduceStream[ncomp];
      tmpReduceStream[ncomp] = NULL;
   }

   if (!tmpReduceStream[ncomp]) {
      int extents[2] = {h,w};
      __BRTStreamType t = (__BRTStreamType) ncomp;
      tmpReduceStream[ncomp] =
         new GLStream(runtime, 1, &t, 2, extents);
   }

   t = tmpReduceStream[ncomp];

   assert (inputReduceStreamTreg == 0 ||
         inputReduceStreamTreg == 1);

   assert (reduceTreg == 0 ||
         reduceTreg == 1);

   /* First reduce in the x dir */
   while (w != 1) {
      int half = w/2;
      int remainder = w%2;
      int wp = half*2;
      glfloat4 f1[2], f2[2];

      ComputeTexCoords(half, h, false, false,
            0.0f, 0.0f, 0.0f, 1.0f,
            (float) wp, (float) h, 0.0f, 1.0f,
            f1[0], f2[0]);

      ComputeTexCoords(half, h, false, false,
            1.0f, 0.0f, 0.0f, 1.0f,
            (float) wp+1, (float) h, 0.0f, 1.0f,
            f1[1], f2[1]);

      if (first) {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
      } else {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      }
      CHECK_GL();

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);
      issue_reduce_poly(0, 0, half, h, 2, f1, f2);

      if (remainder) {
         ComputeTexCoords(1, h, false, false,
               (float) (w-1)+0.5f, 0.0f, 0.0f, 1.0f,
               (float) (w-1)+0.5f, (float) h, 0.0f, 1.0f,
               f1[0], f2[0]);

         glActiveTextureARB(GL_TEXTURE0_ARB);
         if (first)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
         else
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);
         issue_reduce_poly(half, 0, 1, h, 1, f1, f2);
         if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
      }

      first = false;

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
            0, 0, 0, 0,
            half+remainder, h);
      if (sreg0)
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);

      w = half+remainder;
   }

   /* Now reduce in the y dir */
   while (h != 1) {
      int half = h/2;
      int remainder = h%2;
      int hp = half*2;
      glfloat4 f1[2], f2[2];

      ComputeTexCoords(1, half, false, false,
            0.0f,       0.0f, 0.0f, 1.0f,
            0.0f, (float) hp, 0.0f, 1.0f,
            f1[0], f2[0]);

      ComputeTexCoords(1, half, false, false,
            0.0f,         1.0f, 0.0f, 1.0f,
            0.0f, (float) hp+1, 0.0f, 1.0f,
            f1[1], f2[1]);

      if (first) {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
      } else {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      }
      CHECK_GL();

      glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);
      issue_reduce_poly(0, 0, 1, half, 2, f1, f2);

      if (remainder) {
         ComputeTexCoords(1, 1, false, false,
               0.0f, (float) (h-1), 0.0f, 1.0f,
               0.0f, (float) (h-1), 0.0f, 1.0f,
               f1[0], f2[0]);

         glActiveTextureARB(GL_TEXTURE0_ARB);
         if (first)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
         else
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);

         issue_reduce_poly(0, half, 1, 1, 1, f1, f2);
         if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
      }

      first = false;

      if (half + remainder > 1) {
         glActiveTextureARB(GL_TEXTURE0_ARB);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);
         glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
               0, 0, 0, 0,
               1, half+remainder);
         if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
      }
      h = half+remainder;
   }

   glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, &readback);

   for (i=0; i<ncomp; i++)
      ((float *)reduceVal)[i] = ((float *)&readback)[i];
}

void
GLKernel::ReduceStream()
{
   int w = inputReduceStream->width;
   int h = inputReduceStream->height;
   int ncomp = inputReduceStream->ncomp[0];
   int ratiox;
   int ratioy;
   int nx;
   int ny;

   glfloat4 f1[2], f2[2];

   int remainder_x;
   int remainder_y;

   int leftSreg, rightSreg;

   Stream* outputStreamBase = *((const ::brook::stream*) reduceVal);
   GLStream *r = (GLStream *)outputStreamBase;
   GLStream *t;

   bool first = true;
   bool have_remainder = false;

   nx = r->width;
   ny = r->height;

   assert (nx <= w);
   assert (ny <= h);
   assert (w % nx == 0);
   assert (h % ny == 0);

   ratiox = w / nx;
   ratioy = h / ny;

   // Don't attempt reduce on 1x1
   if (w == 1 && h == 1) {
      glfloat4 data;
      inputReduceStream->Write((void *)&data);
      r->Read((void *)&data);
      return;
   }

   // Initialize the pbuffer to the right dimension
   runtime->createPBuffer(ncomp);

   /* Construct the temp stream */
   if (tmpReduceStream[ncomp] &&
         (tmpReduceStream[ncomp]->width < (unsigned int) w ||
          tmpReduceStream[ncomp]->height < (unsigned int) h)) {
      delete tmpReduceStream[ncomp];
      tmpReduceStream[ncomp] = NULL;
   }

   if (!tmpReduceStream[ncomp]) {
      int extents[2] = {h,w};
      __BRTStreamType t = (__BRTStreamType) ncomp;

      tmpReduceStream[ncomp] =
         new GLStream(runtime, 1, &t, 2, extents);
   }
   t = tmpReduceStream[ncomp];

   remainder_x = w/2;
   remainder_y = 0;

   assert (inputReduceStreamTreg == 0 ||
         inputReduceStreamTreg == 1);

   assert (reduceTreg == 0 ||
         reduceTreg == 1);

   assert (treg == 2);

   if (inputReduceStreamSreg < reduceSreg) {
      leftSreg = inputReduceStreamSreg;
      rightSreg = reduceSreg;
   } else {
      leftSreg = reduceSreg;
      rightSreg = inputReduceStreamSreg;
   }

   /* First reduce in the x dir */
   while (ratiox != 1) {
      int half = ratiox/2;
      int remainder = ratiox%2;

      ComputeTexCoords(half*nx, h, false, false,
            0.0f, 0.0f, 0.0f, 1.0f,
            (float) nx*ratiox, (float) h, 0.0f, 1.0f,
            f1[0], f2[0]);

      ComputeTexCoords(half*nx, h, false, false,
            1.0f, 0.0f, 0.0f, 1.0f,
            (float) nx*ratiox+1,  (float) h, 0.0f, 1.0f,
            f1[1], f2[1]);

      if (first) {
         glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
      } else {
         glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
         glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      }

      glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);

      issue_reduce_poly(0, 0, half*nx, h, 2, f1, f2);

      if (remainder) {
        if (have_remainder) {

          assert (!first);

          ComputeTexCoords(nx, h, false, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0]);
          ComputeTexCoords(nx, h, false, false,
                     (float) remainder_x, 0.0f, 0.0f, 1.0f,
                     (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                     f1[1], f2[1]);

          if (first) {
            glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
            glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
          } else {
            glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
            glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
          }

          glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);

          issue_reduce_poly(remainder_x, remainder_y, nx, h, 2, f1, f2);

        } else {
          //  Copy the remainder values over
           ComputeTexCoords(nx, h, false, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0]);

          glActiveTextureARB(GL_TEXTURE0_ARB);
          if (first)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
          else
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
          glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);
          issue_reduce_poly(remainder_x, remainder_y, nx, h, 1, f1, f2);
          if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
          CHECK_GL();
        }

        CHECK_GL();

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                            remainder_x, remainder_y,
                            remainder_x, remainder_y,
                            nx, h);
        if (sreg0)
          glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);

        CHECK_GL();

        have_remainder = true;
      }

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                          0, 0, 0, 0,
                          half*nx, h);
      if (sreg0)
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);

      CHECK_GL();

      first = false;

      w = half * nx;
      ratiox = half;
   }

   /* Perform final remainder calc */
   if (have_remainder) {
      ComputeTexCoords(nx, h, false, false,
                (float) 0.0f, 0.0f, 0.0f, 1.0f,
                (float) nx, (float) h, 0.0f, 1.0f,
                f1[0], f2[0]);
      ComputeTexCoords(nx, h, false, false,
                (float) remainder_x, 0.0f, 0.0f, 1.0f,
                (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                f1[1], f2[1]);

     glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
     glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
     glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);
     issue_reduce_poly(0, 0, nx, h, 2, f1, f2);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                         0, 0, 0, 0,
                         nx, h);

     CHECK_GL();
   }

   /* Yeah, we've reduced in x, now do the same thing in y */
   have_remainder = false;

   while (ratioy != 1) {
      int half = ratioy/2;
      int remainder = ratioy%2;

      ComputeTexCoords(nx, half*ny, false, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy, 0.0f, 1.0f,
                 f1[0], f2[0]);

      ComputeTexCoords(nx, half*ny, false, false,
                 0.0f, 1.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy+1, 0.0f, 1.0f,
                 f1[1], f2[1]);

      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, inputReduceStream->id[0]);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      }

      glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);
      issue_reduce_poly(0, 0, nx, ny*half, 2, f1, f2);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                          0, 0, 0, 0,
                          nx, half*ny);
      if (sreg0)
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);

      CHECK_GL();


      if (remainder) {
        if (have_remainder) {
           ComputeTexCoords(nx, ny, false, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx, (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0]);
           ComputeTexCoords(nx, ny, false, false,
                     (float) 0, 0.0f, 0.0f, 1.0f,
                     (float) nx, (float) ny, 0.0f, 1.0f,
                     f1[1], f2[1]);

          glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
          glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
          glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
          glBindTexture (GL_TEXTURE_RECTANGLE_EXT, r->id[0]);
          glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);
          issue_reduce_poly(0, 0, nx, ny, 2, f1, f2);
        } else {
          //  Copy the remainder values over
           ComputeTexCoords(nx, ny, false, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx,   (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0]);
          glActiveTextureARB(GL_TEXTURE0_ARB);
          glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
          glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);
          issue_reduce_poly(0, 0, nx, ny, 1, f1, f2);
          if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
        }

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_EXT, r->id[0]);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                            0, 0, 0, 0, nx, ny);
        if (sreg0)
          glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);

        CHECK_GL();


        have_remainder = true;
      }

      first = false;

      CHECK_GL();

      h = half * ny;
      ratioy = half;
   }

   if (have_remainder) {
     ComputeTexCoords(nx, ny, false, false, 0.0f, 0.0f, 0.0f, 1.0f,
                        (float) nx, (float) ny, 0.0f, 1.0f, f1[0], f2[0]);
     ComputeTexCoords(nx, ny, false, false, 0.0f, 0.0f, 0.0f, 1.0f,
                        (float) nx, (float) ny, 0.0f, 1.0f, f1[1], f2[1]);

     glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
     glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, r->id[0]);
     glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, pass_id[0]);

     issue_reduce_poly(0, 0, nx, ny, 2, f1, f2);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, nx, h);
   } else {

     //  Copy the values over
     ComputeTexCoords(nx, ny, false, false, 0.0f, 0.0f, 0.0f, 1.0f,
                        (float) nx, (float) ny, 0.0f, 1.0f, f1[0], f2[0]);

     glActiveTextureARB(GL_TEXTURE0_ARB);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, t->id[0]);
     glBindProgramARB (GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);

     issue_reduce_poly(0, 0, nx, ny, 1, f1, f2);

     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, r->id[0]);
     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, nx, ny);
     if (sreg0)
       glBindTexture (GL_TEXTURE_RECTANGLE_EXT, sreg0->id[0]);
   }

   CHECK_GL();
}

void
GLKernel::PushConstant(const float &val)
{
   BindParameter(val, 0.0f, 0.0f, 0.0f);
   creg++;
   argcount++;
}

void
GLKernel::PushConstant(const float2 &val)
{
   BindParameter(val.x, val.y, 0.0f, 0.0f);
   creg++;
   argcount++;
}

void
GLKernel::PushConstant(const float3 &val)
{
   BindParameter(val.x, val.y, val.z, 0.0f);
   creg++;
   argcount++;
}

void
GLKernel::PushConstant(const float4 &val) {
   BindParameter(val.x, val.y, val.z, val.w);
   creg++;
   argcount++;
}
