
#include <stdio.h>

#include "nv30gl.hpp"

using namespace brook;

static const char passthrough[] =        \
"!!FP1.0\n"                              \
"TEX  R0, f[TEX0].xyyy, TEX0, RECT;\n"   \
"MOVR o[COLR], R0;\n"                    \
"END\n";

static GLuint passthrough_id = 0;

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

   for (i=0; i<5; i++)
      tmpReduceStream[i] = NULL;
   
   if (!passthrough_id) {
      glGenProgramsNV (1, &passthrough_id);
      glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, 
                       passthrough_id, strlen(passthrough), 
                       (const GLubyte*) passthrough);
      CHECK_GL();
   }

   /* Initialize state machine */
   creg = 1;
   sreg = 0;
   treg = 0;
   outstream = NULL;

   for (i=0; i<NV30GL_MAX_TEXCOORDS; i++) {
      sargs[i] = NULL;
      iargs[i] = NULL;
   }
}

void NV30GLKernel::PushStream(Stream *s) {
   
   NV30GLStream *nv_stream = (NV30GLStream *) s;

   glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, nv_stream->id);
   CHECK_GL();

   inputReduceStream = nv_stream;
   inputReduceStreamTreg = treg;
   inputReduceStreamSreg = sreg;

   sargs[treg] = nv_stream;
   sreg++;
   treg++;
}

void NV30GLKernel::PushIter(Iter *s) {

   NV30GLIter *nv_iter = (NV30GLIter *) s;

   iargs[treg] = nv_iter;
   treg++;
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
void NV30GLKernel::PushReduce(void * val, __BRTStreamType type) {
 
   reduceVal = val;
   reduceType = type;
   reduceSreg = sreg;
   reduceTreg = treg;

   sreg++;
   treg++;
}

void NV30GLKernel::PushGatherStream(Stream *s) {
   glActiveTextureARB(GL_TEXTURE0_ARB+sreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, ((NV30GLStream *)s)->id);
   CHECK_GL();
   
   sreg++;
   creg++;
}

void NV30GLKernel::PushOutput(Stream *s) {
   assert (outstream == NULL);
   outstream = (NV30GLStream *) s;
}

static void
compute_st(unsigned int w, unsigned int h,
           bool is_1D_iter,
           float x1, float y1, float z1, float w1,
           float x2, float y2, float z2, float w2,
           float4 &f1, float4 &f2, float4 &fd) {

   const float half_pixel  = 0.5f / NV30GLRunTime::workspace;
 
   if (is_1D_iter) {
      // 1D
      if (w == 1) {
         f1.x = x1; f1.y = y1;
         f1.z = z1; f1.w = w1;
         f2 = f1;
         fd.x = 0.0f; fd.y = 0.0f;
         fd.z = 0.0f; fd.w = 0.0f;
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
         f1.x = x1 - shiftx + half_pixel;
         f1.y = y1 - shifty + half_pixel;
         f1.z = z1 - shiftz + half_pixel;
         f1.w = w1 - shiftw + half_pixel;
         f2.x = x2 - shiftx + half_pixel;
         f2.y = y2 - shifty + half_pixel;
         f2.z = z2 - shiftz + half_pixel;
         f2.w = w2 - shiftw + half_pixel;
         fd.x = sx;
         fd.y = sy;
         fd.z = sz;
         fd.w = sw;
      }
   } else {
      // 2D
      if (w == 1) {
         f1.x = x1 + half_pixel;
         f2.x = f1.x;
         fd.x = 0.0f;
      } else {
         float sw = x2-x1;
         float ratio = sw / w;
         float shift = ratio * 0.5f;
         f1.x = x1 - shift + half_pixel;
         f2.x = x2 - shift + half_pixel;
         fd.x = sw;
      }

      if (h == 1) {
         f1.y = y1 + half_pixel;
         f2.y = f1.y;
         fd.y = 0.0f;
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
         f1.y = y1 - shift + half_pixel * (ratio*2+1);
         f2.y = y2 - shift + half_pixel * (ratio*2+1);
         fd.y = sh;
      }
      f1.z = 0.0f;
      f1.w = 1.0f;
      f2.z = 0.0f;
      f2.w = 1.0f;
      fd.z = 0.0f;
      fd.w = 0.0f;
   }

#if 0
   fprintf (stderr,
            "\t%3.8f %3.8f %3.8f %3.8f\n\t%3.8f %3.8f %3.8f %3.8f\n", 
            f1.x, f1.y, f1.z, f1.w,
            f2.x, f2.y, f2.z, f2.w);
#endif


}

void NV30GLKernel::Map() {
   
   unsigned int i;
   int w, h;
   float4 f1[8], f2[8];
   float4 fd[8];

   if (outstream == NULL) {
      fprintf (stderr, "No output stream specified for kernel\n");
      exit(1);
   }
   
   w = outstream->width;
   h = outstream->height;

   /* Compute texture coordinates */
   for (i=0; i<8; i++) {
      if (sargs[i])
         compute_st(w, h, false,
                    0.0f, 0.0f, 0.0f, 1.0f,
                    (float) sargs[i]->width, 
                    (float) sargs[i]->height, 0.0f, 1.0f, 
                    f1[i], f2[i], fd[i]);
      else if (iargs[i]) {
         NV30GLIter *itr = iargs[i]; 
         compute_st(w, h, (itr->dims==1),
                    itr->min.x, itr->min.y,
                    itr->min.z, itr->min.w,
                    itr->max.x, itr->max.y,
                    itr->max.z, itr->max.w,
                    f1[i], f2[i], fd[i]);
      }
   }

#if 0
   fprintf (stderr, "w: %d, h: %d\n", w, h);
   for (i=0; i<8; i++)
      if (sargs[i] || iargs[i])
         fprintf (stderr,
                  "%d:\t%3.8f %3.8f %3.8f %3.8f\n\t%3.8f %3.8f %3.8f %3.8f\n", 
                  i, f1[i].x, f1[i].y, f1[i].z, f1[i].w,
                  f2[i].x, f2[i].y, f2[i].z, f2[i].w);
#endif

   CHECK_GL();

   glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);

   glViewport(0,0,w,h);

   /* Issue the poly */
   glBegin(GL_TRIANGLES);
   
   for (i=0; i<8; i++)
      if (sargs[i] || (iargs[i] && iargs[i]->dims==2))
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f1[i].x, f2[i].y+fd[i].y, 0.0f, 1.0f);
      else if (iargs[i] && iargs[i]->dims==1) 
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f1[i].x, f1[i].y, f1[i].z, f1[i].w);
   glVertex2f(-1.0f, 3.0f); 

   for (i=0; i<8; i++)
      if (sargs[i] || (iargs[i] && iargs[i]->dims==2))
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f1[i].x, f1[i].y, 0.0f, 1.0f);
      else if (iargs[i] && iargs[i]->dims==1) 
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f1[i].x, f1[i].y, f1[i].z, f1[i].w);
   glVertex2f(-1.0f, -1.0f);
   
   for (i=0; i<8; i++)
      if (sargs[i] || (iargs[i] && iargs[i]->dims==2))
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f2[i].x+fd[i].x, f1[i].y, 0.0f, 1.0f);
      else if (iargs[i] && iargs[i]->dims==1) 
         glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                              f2[i].x+fd[i].x, 
                              f2[i].y+fd[i].y,
                              f2[i].z+fd[i].z,
                              f2[i].w+fd[i].w);
   glVertex2f(3.0f, -1.0f);

   glEnd();
   CHECK_GL();

   glBindTexture (GL_TEXTURE_RECTANGLE_NV, outstream->id);
   glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                    GLtype[outstream->ncomp], 0, 0, w, h, 0);
   CHECK_GL();

   /* Reset state machine */
   creg = 1;
   sreg = 0;
   treg = 0;
   for (i=0; i<NV30GL_MAX_TEXCOORDS; i++) {
      sargs[i] = NULL;
      iargs[i] = NULL;
   }
   outstream = NULL;
}


void
NV30GLKernel::Reduce() {
   assert (inputReduceStream);

   if (reduceType == __BRTSTREAM)
      ReduceStream();
   else
      ReduceScalar();

   /* Reset state machine */
   creg = 1;
   sreg = 0;
   treg = 0;
   reduceVal = NULL;
   inputReduceStream = NULL;
}

static void
issue_reduce_poly (int x1, int y1, 
                   int w, int h, int n,
                   float4 f1[], float4 f2[], float4 fd[]) {
   
   int i;

   glViewport(x1,y1,w,h);
   
   /* Issue the poly */
   glBegin(GL_TRIANGLES);
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                           f1[i].x, f2[i].y+fd[i].y, 0.0f, 1.0f);
   glVertex2f(-1.0f, 3.0f); 
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                           f1[i].x, f1[i].y, 0.0f, 1.0f);
   glVertex2f(-1.0f, -1.0f);
   for (i=0; i<n; i++)
      glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i, 
                           f2[i].x+fd[i].x, f1[i].y, 0.0f, 1.0f);
   glVertex2f(3.0f, -1.0f);
   glEnd();
   CHECK_GL();
}

#if 0
static void
dumpframebuffer (int w, int h) {
   int i, j;
   float4 *pix = (float4 *) malloc (w*h*sizeof(float4));
   glReadPixels (0,0,w,h,GL_RGBA,GL_FLOAT,pix);

   fprintf (stderr, "---------\n");
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++)
         fprintf(stderr,"%3.2f ", pix[i*w+j].x);
      fprintf(stderr, "\n");
   }
   fprintf (stderr, "---------\n");
}
#else
#define dumpframebuffer(a,b)
#endif


void
NV30GLKernel::ReduceScalar() {
   /* First reduce in the x direction */
   int i;
   int w = inputReduceStream->width;
   int h = inputReduceStream->height;
   int ncomp = inputReduceStream->ncomp;
   float4 readback;
   
   bool first = true;

   // Don't attempt reduce on 1x1
   if (w == 1 && h == 1) {
      inputReduceStream->GLReadData(reduceVal);
      return;
   }

   /* Construct the temp stream */
   if (tmpReduceStream[ncomp] && 
       (tmpReduceStream[ncomp]->width < (unsigned int) w ||
        tmpReduceStream[ncomp]->height < (unsigned int) h)) {
     delete tmpReduceStream[ncomp];
     tmpReduceStream[ncomp] = NULL;
   }

   if (!tmpReduceStream[ncomp]) {
     int extents[2] = {w,h};
      tmpReduceStream[ncomp] = 
         new NV30GLStream(runtime, (__BRTStreamType) ncomp, 2, extents);
   }

   assert (inputReduceStreamTreg == 0 ||
           inputReduceStreamTreg == 1);

   assert (reduceTreg == 0 ||
           reduceTreg == 1);

   glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
   CHECK_GL();

   glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
   CHECK_GL();

   /* First reduce in the x dir */
   while (w != 1) {
      int half = w/2;
      int remainder = w%2;
      int wp = half*2;
      float4 f1[2], f2[2], fd[2];
   
      compute_st(half, h, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) wp, (float) h, 0.0f, 1.0f,
                 f1[0], f2[0], fd[0]);

      compute_st(half, h, false,
                 1.0f, 0.0f, 0.0f, 1.0f,
                 (float) wp+1, (float) h, 0.0f, 1.0f,
                 f1[1], f2[1], fd[1]);
         
      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
      issue_reduce_poly(0, 0, half, h, 2, f1, f2, fd);
      
      if (remainder) {
         compute_st(1, h, false,
                    (float) (w-1)+0.5f, 0.0f, 0.0f, 1.0f,
                    (float) (w-1)+0.5f, (float) h, 0.0f, 1.0f,
                    f1[0], f2[0], fd[0]);
         
#if 0
         fprintf (stderr,
                  "\t%3.8f %3.8f %3.8f %3.8f\n\t%3.8f %3.8f %3.8f %3.8f\n", 
                  f1[0].x, f1[0].y, f1[0].z, f1[0].w,
                  f2[0].x, f2[0].y, f2[0].z, f2[0].w);
#endif

         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, passthrough_id);
         issue_reduce_poly(half, 0, 1, h, 1, f1, f2, fd);
      }


      if (first) {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                        tmpReduceStream[ncomp]->id);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                        tmpReduceStream[ncomp]->id);
         CHECK_GL();
         first = false;
      }

      //  dumpframebuffer(10, 10);

      glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                       GLtype[ncomp], 0, 0, 
                       half+remainder, h, 0);

      w = half+remainder;
   }

   /* Now reduce in the y dir */
   while (h != 1) {
      int half = h/2;
      int remainder = h%2;
      int hp = half*2;
      float4 f1[2], f2[2], fd[2];
   
      compute_st(1, half, false,
                 0.0f,       0.0f, 0.0f, 1.0f,
                 0.0f, (float) hp, 0.0f, 1.0f,
                 f1[0], f2[0], fd[0]);

      compute_st(1, half, false,
                 0.0f,         1.0f, 0.0f, 1.0f,
                 0.0f, (float) hp+1, 0.0f, 1.0f,
                 f1[1], f2[1], fd[1]);
         
      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
      issue_reduce_poly(0, 0, 1, half, 2, f1, f2, fd);
      
      if (remainder) {
         compute_st(1, 1, false,
                    0.0f, (float) (h-1), 0.0f, 1.0f,
                    0.0f, (float) (h-1), 0.0f, 1.0f,
                    f1[0], f2[0], fd[0]);
         
         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, passthrough_id);
         issue_reduce_poly(0, half, 1, 1, 1, f1, f2, fd);
      }


      if (first) {
         glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                        tmpReduceStream[ncomp]->id);
         glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
         glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                        tmpReduceStream[ncomp]->id);
         CHECK_GL();
         first = false;
      }

      if (half + remainder > 1) {
         glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                          GLtype[ncomp], 0, 0, 
                          1, half+remainder, 0);
      }

      h = half+remainder;
   }
         
   glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, &readback);
   
   for (i=0; i<ncomp; i++)
      ((float *)reduceVal)[i] = ((float *)&readback)[i];

}

void
NV30GLKernel::ReduceStream() {

   int w = inputReduceStream->width;
   int h = inputReduceStream->height;
   int ncomp = inputReduceStream->ncomp;
   int ratiox;
   int ratioy;
   int nx;
   int ny;

   float4 f1[2], f2[2], fd[2];

   int remainder_x;
   int remainder_y;
   
   Stream* outputStreamBase = *((const __BRTStream*) reduceVal);
   NV30GLStream *r = (NV30GLStream *)outputStreamBase;
   NV30GLStream *t;

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
      float4 data;
      inputReduceStream->GLReadData((void *)&data);
      r->GLWriteData((void *)&data);
      return;
   }

   /* Construct the temp stream */
   if (tmpReduceStream[ncomp] && 
       (tmpReduceStream[ncomp]->width < (unsigned int) w ||
        tmpReduceStream[ncomp]->height < (unsigned int) h)) {
     delete tmpReduceStream[ncomp];
     tmpReduceStream[ncomp] = NULL;
   }
   
   if (!tmpReduceStream[ncomp]) {
     int extents[2] = {w,h};

     tmpReduceStream[ncomp] = 
       new NV30GLStream(runtime, (__BRTStreamType) ncomp, 2, extents);
   }
   t = tmpReduceStream[ncomp];

   remainder_x = w/2;
   remainder_y = 0;

   assert (inputReduceStreamTreg == 0 ||
           inputReduceStreamTreg == 1);

   assert (reduceTreg == 0 ||
           reduceTreg == 1);

   /* First reduce in the x dir */
   while (ratiox != 1) {
      int half = ratiox/2;
      int remainder = ratiox%2;
      int ratioxp = half*2;
   
      compute_st(half*nx, h, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx*ratiox, (float) h, 0.0f, 1.0f,
                 f1[0], f2[0], fd[0]);

      compute_st(half*nx, h, false,
                 1.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx*ratiox+1,  (float) h, 0.0f, 1.0f,
                 f1[1], f2[1], fd[1]);

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
      }

      glClearColor(555.555f, 555.555f, 555.555f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      issue_reduce_poly(0, 0, half*nx, h, 2, f1, f2, fd);

      dumpframebuffer(10,10);

      if (remainder) { 
        if (have_remainder) {
          compute_st(nx, h, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0], fd[0]);
          compute_st(nx, h, false,
                     (float) remainder_x, 0.0f, 0.0f, 1.0f,
                     (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                     f1[1], f2[1], fd[1]);
          
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
          issue_reduce_poly(remainder_x, remainder_y, nx, h, 2, f1, f2, fd);
        } else {
          //  Copy the remainder values over
          compute_st(nx, h, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0], fd[0]);

         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, passthrough_id);
         issue_reduce_poly(remainder_x, remainder_y, nx, h, 1, f1, f2, fd);
        }

        dumpframebuffer(10,10);
        
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                       t->id);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                            remainder_x, remainder_y,
                            remainder_x, remainder_y, 
                            nx, h);

        have_remainder = true;

      }


      glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                     t->id);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                          0, 0, 0, 0, 
                          half*nx, h);

      first = false;
      CHECK_GL();

      w = half * nx;
      ratiox = half;
   }

   if (have_remainder) {
     compute_st(nx, h, false,
                (float) 0.0f, 0.0f, 0.0f, 1.0f,
                (float) nx, (float) h, 0.0f, 1.0f,
                f1[0], f2[0], fd[0]);
     compute_st(nx, h, false,
                (float) remainder_x, 0.0f, 0.0f, 1.0f,
                (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                f1[1], f2[1], fd[1]);
     
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
     issue_reduce_poly(0, 0, nx, h, 2, f1, f2, fd);

     dumpframebuffer(10,10);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                         0, 0, 0, 0, 
                         nx, h);
   }

   /* Yeah, we've reduced in x, now do the same thing in y */
   have_remainder = false;

   while (ratioy != 1) {
      int half = ratioy/2;
      int remainder = ratioy%2;
      int ratioyp = half*2;
   
      compute_st(nx, half*ny, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy, 0.0f, 1.0f,
                 f1[0], f2[0], fd[0]);

      compute_st(nx, half*ny, false,
                 0.0f, 1.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy+1, 0.0f, 1.0f,
                 f1[1], f2[1], fd[1]);

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
      }

      glClearColor(555.555f, 555.555f, 555.555f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      issue_reduce_poly(0, 0, nx, ny*half, 2, f1, f2, fd);

      dumpframebuffer(10,10);

      glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                     t->id);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                          0, 0, 0, 0, 
                          nx, half*ny);

      if (remainder) { 
        if (have_remainder) {
          compute_st(nx, ny, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx, (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0], fd[0]);
          compute_st(nx, ny, false,
                     (float) 0, 0.0f, 0.0f, 1.0f,
                     (float) nx, (float) ny, 0.0f, 1.0f,
                     f1[1], f2[1], fd[1]);
          
          glActiveTextureARB(GL_TEXTURE0_ARB);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
          glActiveTextureARB(GL_TEXTURE1_ARB);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id);
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);
          issue_reduce_poly(0, 0, nx, ny, 2, f1, f2, fd);
        } else {
          //  Copy the remainder values over
          compute_st(nx, ny, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx,   (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0], fd[0]);
          glActiveTextureARB(GL_TEXTURE0_ARB);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, passthrough_id);
          issue_reduce_poly(0, 0, nx, ny, 1, f1, f2, fd);
        }

        dumpframebuffer(10,10);
        
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                       r->id);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                            0, 0, 0, 0, 
                            nx, ny);

        have_remainder = true;
      }


      first = false;
      CHECK_GL();

      h = half * ny;
      ratioy = half;
   }

   if (have_remainder) {
     compute_st(nx, ny, false,
                      0.0f,       0.0f, 0.0f, 1.0f,
                (float) nx, (float) ny, 0.0f, 1.0f,
                f1[0], f2[0], fd[0]);
     compute_st(nx, ny, false,
                      0.0f,       0.0f, 0.0f, 1.0f,
                (float) nx, (float) ny, 0.0f, 1.0f,
                f1[1], f2[1], fd[1]);
     
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);

     glActiveTextureARB(GL_TEXTURE0_ARB);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
     glActiveTextureARB(GL_TEXTURE1_ARB);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id);

     issue_reduce_poly(0, 0, nx, ny, 2, f1, f2, fd);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                         0, 0, 0, 0, 
                         nx, h);
   } else {

     //  Copy the values over
     compute_st(nx, ny, false,
                0.0f, 0.0f, 0.0f, 1.0f,
                (float) nx,   (float) ny, 0.0f, 1.0f,
                f1[0], f2[0], fd[0]);
     glActiveTextureARB(GL_TEXTURE0_ARB);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id);
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, passthrough_id);

     issue_reduce_poly(0, 0, nx, ny, 1, f1, f2, fd);

     glBindTexture (GL_TEXTURE_RECTANGLE_NV, 
                    r->id);
     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                         0, 0, 0, 0, 
                         nx, ny);
   }

   dumpframebuffer(10,10);

}


NV30GLKernel::~NV30GLKernel() {
   unsigned int i;
 
   for (i=0; i<NV30GL_MAXCONSTS; i++)
      free(constnames[i]);

   for (i=0; i<5; i++)
      if (tmpReduceStream[i])
         delete (tmpReduceStream[i]);

}
