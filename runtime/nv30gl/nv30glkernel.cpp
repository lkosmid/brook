
#include "nv30gl.hpp"

using namespace brook;

NV30GLKernel::NV30GLKernel(NV30GLRunTime * runtime,
                           const void *sourcelist[]) :
   GLKernel(runtime, sourcelist)
{
}


static void
compute_st(unsigned int w, unsigned int h,
           bool is_1D_iter,
           float x1, float y1, float z1, float w1,
           float x2, float y2, float z2, float w2,
           glfloat4 &f1, glfloat4 &f2)
{

   const float half_pixel  = 0.5f / NV30GLRunTime::workspace;
   const float pixel  = 1.0f / NV30GLRunTime::workspace;

   if (is_1D_iter) {
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

#if 0
   fprintf (stderr,
            "\t%3.8f %3.8f %3.8f %3.8f\n\t%3.8f %3.8f %3.8f %3.8f\n",
            f1.x, f1.y, f1.z, f1.w,
            f2.x, f2.y, f2.z, f2.w);
#endif


}

void NV30GLKernel::Map() {

   unsigned int i, j, scount;
   int w, h;
   glfloat4 f1[8], f2[8];

   w = -1; h = -1; scount = 0;

   for (j=0; j<npasses; ) {

     if (w != (int) outstream[scount]->width ||
         h != (int) outstream[scount]->height) {

       // Compute texture coords

       w = outstream[scount]->width;
       h = outstream[scount]->height;

       /* Compute texture coordinates */
       for (i=0; i<8; i++) {
         if (sargs[i])
           compute_st(w, h, false,
                      0.0f, 0.0f, 0.0f, 1.0f,
                      (float) sargs[i]->width,
                      (float) sargs[i]->height, 0.0f, 1.0f,
                      f1[i], f2[i]);
         else if (iargs[i]) {
           NV30GLIter *itr = (NV30GLIter *) iargs[i];
           compute_st(w, h, (itr->dims==1),
                      itr->min.x, itr->min.y,
                      itr->min.z, itr->min.w,
                      itr->max.x, itr->max.y,
                      itr->max.z, itr->max.w,
                      f1[i], f2[i]);
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

       glViewport(0,0,w,h);
     }

     unsigned int nfields = outstream[scount]->getFieldCount();
     NV30GLStream *outp = (NV30GLStream *) outstream[scount];
     for (unsigned int k=0; k < (unsigned int) nfields; k++) {
        if (runtime->pbuffer_ncomp != outp->ncomp[k])
           runtime->createPBuffer(outp->ncomp[k]);

        glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[j]);

        /* Issue the poly */
        glBegin(GL_TRIANGLES);

        for (i=0; i<8; i++)
           if (sargs[i] || (iargs[i] && iargs[i]->dims==2))
              glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
                                   f1[i].x, f2[i].y, 0.0f, 1.0f);
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
                                   f2[i].x, f1[i].y, 0.0f, 1.0f);
           else if (iargs[i] && iargs[i]->dims==1)
              glMultiTexCoord4fARB(GL_TEXTURE0_ARB+i,
                                   f2[i].x,
                                   f2[i].y,
                                   f2[i].z,
                                   f2[i].w);
        glVertex2f(3.0f, -1.0f);

        glEnd();
        CHECK_GL();

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, outp->id[k]);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, w, h);
        if (sreg0)
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
        CHECK_GL();

        j++;
     }
	 scount++;
   }

   ResetStateMachine();
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
NV30GLKernel::ReduceScalar() {
   /* First reduce in the x direction */
   int i;
   int w = inputReduceStream->width;
   int h = inputReduceStream->height;
   int ncomp = inputReduceStream->ncomp[0];
   glfloat4 readback;
   NV30GLStream *t;
   bool first = true;

   assert(npasses == 1);

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
     int extents[2] = {h,w};
     __BRTStreamType t = (__BRTStreamType) ncomp;
      tmpReduceStream[ncomp] =
         new NV30GLStream((NV30GLRunTime *) runtime, 1, &t, 2, extents);
   }

   t = (NV30GLStream *) tmpReduceStream[ncomp];

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

      compute_st(half, h, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) wp, (float) h, 0.0f, 1.0f,
                 f1[0], f2[0]);

      compute_st(half, h, false,
                 1.0f, 0.0f, 0.0f, 1.0f,
                 (float) wp+1, (float) h, 0.0f, 1.0f,
                 f1[1], f2[1]);

      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      }
      CHECK_GL();

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);
      issue_reduce_poly(0, 0, half, h, 2, f1, f2);

      if (remainder) {
         compute_st(1, h, false,
                    (float) (w-1)+0.5f, 0.0f, 0.0f, 1.0f,
                    (float) (w-1)+0.5f, (float) h, 0.0f, 1.0f,
                    f1[0], f2[0]);

         glActiveTextureARB(GL_TEXTURE0_ARB);
         if (first)
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
         else
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);
         issue_reduce_poly(half, 0, 1, h, 1, f1, f2);
         if (sreg0)
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
      }

      first = false;

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                       0, 0, 0, 0,
                       half+remainder, h);
      if (sreg0)
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);

      w = half+remainder;
   }

   /* Now reduce in the y dir */
   while (h != 1) {
      int half = h/2;
      int remainder = h%2;
      int hp = half*2;
      glfloat4 f1[2], f2[2];

      compute_st(1, half, false,
                 0.0f,       0.0f, 0.0f, 1.0f,
                 0.0f, (float) hp, 0.0f, 1.0f,
                 f1[0], f2[0]);

      compute_st(1, half, false,
                 0.0f,         1.0f, 0.0f, 1.0f,
                 0.0f, (float) hp+1, 0.0f, 1.0f,
                 f1[1], f2[1]);

      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB+inputReduceStreamSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+reduceSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      }
      CHECK_GL();

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);
      issue_reduce_poly(0, 0, 1, half, 2, f1, f2);

      if (remainder) {
         compute_st(1, 1, false,
                    0.0f, (float) (h-1), 0.0f, 1.0f,
                    0.0f, (float) (h-1), 0.0f, 1.0f,
                    f1[0], f2[0]);

         glActiveTextureARB(GL_TEXTURE0_ARB);
         if (first)
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
         else
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);

         issue_reduce_poly(0, half, 1, 1, 1, f1, f2);
         if (sreg0)
           glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
      }

      first = false;

      if (half + remainder > 1) {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                         0, 0, 0, 0,
                         1, half+remainder);
        if (sreg0)
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
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
      glfloat4 data;
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
     int extents[2] = {h,w};
     __BRTStreamType t = (__BRTStreamType) ncomp;

     tmpReduceStream[ncomp] =
         new NV30GLStream((NV30GLRunTime *) runtime, 1, &t, 2, extents);
   }
   t = (NV30GLStream *) tmpReduceStream[ncomp];

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

   clearframebuffer();

   /* First reduce in the x dir */
   while (ratiox != 1) {
      int half = ratiox/2;
      int remainder = ratiox%2;

      compute_st(half*nx, h, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx*ratiox, (float) h, 0.0f, 1.0f,
                 f1[0], f2[0]);

      compute_st(half*nx, h, false,
                 1.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx*ratiox+1,  (float) h, 0.0f, 1.0f,
                 f1[1], f2[1]);

      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      }

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);

      issue_reduce_poly(0, 0, half*nx, h, 2, f1, f2);

      if (remainder) {
        if (have_remainder) {

          assert (!first);

          compute_st(nx, h, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0]);
          compute_st(nx, h, false,
                     (float) remainder_x, 0.0f, 0.0f, 1.0f,
                     (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                     f1[1], f2[1]);

          if (first) {
            glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
            glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
          } else {
            glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
            glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
          }

          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);

          issue_reduce_poly(remainder_x, remainder_y, nx, h, 2, f1, f2);

        } else {
          //  Copy the remainder values over
          compute_st(nx, h, false,
                     (float) ratiox-1, 0.0f, 0.0f, 1.0f,
                     (float) w+ratiox, (float) h, 0.0f, 1.0f,
                     f1[0], f2[0]);

          glActiveTextureARB(GL_TEXTURE0_ARB);
          if (first)
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
          else
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);
          issue_reduce_poly(remainder_x, remainder_y, nx, h, 1, f1, f2);
          if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
          CHECK_GL();
        }

        CHECK_GL();

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                            remainder_x, remainder_y,
                            remainder_x, remainder_y,
                            nx, h);
        if (sreg0)
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);

        CHECK_GL();

        have_remainder = true;
      }

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                          0, 0, 0, 0,
                          half*nx, h);
      if (sreg0)
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);

      CHECK_GL();

      dumpframebuffer(10,10);
      clearframebuffer();

      first = false;

      w = half * nx;
      ratiox = half;
   }

   /* Perform final remainder calc */
   if (have_remainder) {
     compute_st(nx, h, false,
                (float) 0.0f, 0.0f, 0.0f, 1.0f,
                (float) nx, (float) h, 0.0f, 1.0f,
                f1[0], f2[0]);
     compute_st(nx, h, false,
                (float) remainder_x, 0.0f, 0.0f, 1.0f,
                (float) remainder_x+nx, (float) h, 0.0f, 1.0f,
                f1[1], f2[1]);

     glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
     glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);
     issue_reduce_poly(0, 0, nx, h, 2, f1, f2);

     dumpframebuffer(10,10);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                         0, 0, 0, 0,
                         nx, h);

     CHECK_GL();
   }

   /* Yeah, we've reduced in x, now do the same thing in y */
   have_remainder = false;

   while (ratioy != 1) {
      int half = ratioy/2;
      int remainder = ratioy%2;

      compute_st(nx, half*ny, false,
                 0.0f, 0.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy, 0.0f, 1.0f,
                 f1[0], f2[0]);

      compute_st(nx, half*ny, false,
                 0.0f, 1.0f, 0.0f, 1.0f,
                 (float) nx, (float) ny*ratioy+1, 0.0f, 1.0f,
                 f1[1], f2[1]);

      if (first) {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, inputReduceStream->id[0]);
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
        glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      }

      glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);
      issue_reduce_poly(0, 0, nx, ny*half, 2, f1, f2);

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
      glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                          0, 0, 0, 0,
                          nx, half*ny);
      if (sreg0)
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);

      CHECK_GL();


      if (remainder) {
        if (have_remainder) {
          compute_st(nx, ny, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx, (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0]);
          compute_st(nx, ny, false,
                     (float) 0, 0.0f, 0.0f, 1.0f,
                     (float) nx, (float) ny, 0.0f, 1.0f,
                     f1[1], f2[1]);

          glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
          glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id[0]);
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);
          issue_reduce_poly(0, 0, nx, ny, 2, f1, f2);
        } else {
          //  Copy the remainder values over
          compute_st(nx, ny, false,
                     0.0f, (float) ratioy-1, 0.0f, 1.0f,
                     (float) nx,   (float) h+ratioy, 0.0f, 1.0f,
                     f1[0], f2[0]);
          glActiveTextureARB(GL_TEXTURE0_ARB);
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
          glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);
          issue_reduce_poly(0, 0, nx, ny, 1, f1, f2);
          if (sreg0)
            glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
        }

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id[0]);
        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                            0, 0, 0, 0, nx, ny);
        if (sreg0)
          glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);

        CHECK_GL();


        have_remainder = true;
      }

      first = false;

      dumpframebuffer(10,10);
      clearframebuffer();

      CHECK_GL();

      h = half * ny;
      ratioy = half;
   }

   if (have_remainder) {
     compute_st(nx, ny, false,
                      0.0f,       0.0f, 0.0f, 1.0f,
                (float) nx, (float) ny, 0.0f, 1.0f,
                f1[0], f2[0]);
     compute_st(nx, ny, false,
                      0.0f,       0.0f, 0.0f, 1.0f,
                (float) nx, (float) ny, 0.0f, 1.0f,
                f1[1], f2[1]);

     glActiveTextureARB(GL_TEXTURE0_ARB+leftSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
     glActiveTextureARB(GL_TEXTURE0_ARB+rightSreg);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id[0]);
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, pass_id[0]);

     issue_reduce_poly(0, 0, nx, ny, 2, f1, f2);

     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                         0, 0, 0, 0,
                         nx, h);
   } else {

     //  Copy the values over
     compute_st(nx, ny, false,
                0.0f, 0.0f, 0.0f, 1.0f,
                (float) nx,   (float) ny, 0.0f, 1.0f,
                f1[0], f2[0]);

     glActiveTextureARB(GL_TEXTURE0_ARB);
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, t->id[0]);
     glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);

     issue_reduce_poly(0, 0, nx, ny, 1, f1, f2);

     glBindTexture (GL_TEXTURE_RECTANGLE_NV, r->id[0]);
     glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                         0, 0, 0, 0,
                         nx, ny);
     if (sreg0)
       glBindTexture (GL_TEXTURE_RECTANGLE_NV, sreg0->id[0]);
   }

   dumpframebuffer(10,10);

   CHECK_GL();
}
