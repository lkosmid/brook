#include <stdio.h>
#include <stdlib.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLStream::NV30GLStream (NV30GLRunTime *rt, int fieldCount,
                            const __BRTStreamType fieldType[],
                            int dims, const int extents[])
  : GLStream(rt, fieldCount, fieldType, dims, extents)
{
}


void
NV30GLStream::GLReadData (void *data)
{
   float *p = (float *) data;
   unsigned int i;

   for (i=0; i<nfields; i++) {

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture (GL_TEXTURE_RECTANGLE_NV, id[i]);

      if (ncomp[i] == 2) {
         float4 *t = (float4 *) malloc (sizeof(float)*4*width*height);
         glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA,
                       GL_FLOAT, t);

         if (nfields == 1) {
            for (unsigned int j=0; j<width*height; j++) {
               ((float2 *) data)[j].x = t[j].x;
               ((float2 *) data)[j].y = t[j].y;
            }
         } else
            for (unsigned int j=0; j<width*height; j++) {
               ((float2 *) (p+stride[i]))[j].x = t[j].x;
               ((float2 *) (p+stride[i]))[j].y = t[j].y;
            }

         free (t);
      } else {
         float *t;
         if ( nfields == 1)
            t = p;
         else
            t = (float *) malloc (sizeof(float)*ncomp[i]*width*height);


         if (width == 1 && height == 1) {
            glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp[i]],
                          GL_FLOAT, (void *)(t));
            return;
         }
#if 1

         glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, runtime->passthrough_id);

         glViewport (0, 0, width, height);
         if (height == 1) {
            glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0f, 0.5f);
            glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(width*2.0f, 0.5f);
            glVertex2f(3.0f, -1.0f);
            glTexCoord2f(0.0f, 0.5f);
            glVertex2f(-1.0f, 3.0f);
            glEnd();
         } else if (width == 1) {
            glBegin(GL_TRIANGLES);
            glTexCoord2f(0.5f, 0.0f);
            glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(0.5f, 0.0f);
            glVertex2f(3.0f, -1.0f);
            glTexCoord2f(0.5f, height*2.0f);
            glVertex2f(-1.0f, 3.0f);
            glEnd();
         } else {
            glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(width*2.0f, 0.0f);
            glVertex2f(3.0f, -1.0f);
            glTexCoord2f(0.0f, height*2.0f);
            glVertex2f(-1.0f, 3.0f);
            glEnd();
         }
         glFinish();
         glReadPixels (0, 0, width, height, GLformat[ncomp[i]],
                       GL_FLOAT, t);
#else
         glFinish();
         glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp[i]],
                       GL_FLOAT, t);
#endif

         if (nfields != 1) {
            unsigned int j,k;
            float *src = t;
            float *dst = p+stride[i];
            for (j=0; j<width*height; j++) {
               for (k=0; k<ncomp[i]; k++)
                  *dst++ = *src++;
               dst += elemsize-k;
            }
            free(t);
         }
      }
    }
    CHECK_GL();
}

void
NV30GLStream::GLWriteData (const void *data)
{
   unsigned int i;
   float *p = (float*) data;

   glActiveTextureARB(GL_TEXTURE0_ARB);

   for (i=0; i<nfields; i++) {

      glBindTexture (GL_TEXTURE_RECTANGLE_NV, id[i]);

      if (ncomp[i] == 2) {
         float4 *t = (float4 *) malloc (sizeof(float)*4*width*height);
         for (unsigned int j=0; j<width*height; j++) {
            t[j].x = ((float2 *) (p+stride[i]))[j].x;
            t[j].y = ((float2 *) (p+stride[i]))[j].y;
            t[j].z = 0.0f;
            t[j].w = 0.0f;
         }

         glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                         0, 0, width, height, GL_RGBA,
                         GL_FLOAT, t);

         free (t);
      } else {

         if (nfields == 1)
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                            0, 0, width, height, GLformat[ncomp[i]],
                            GL_FLOAT, data);
         else {
            float *t = (float *) malloc (sizeof(float)*ncomp[i]*width*height);
            float *src = p+stride[i];
            float *dst = t;
            for (unsigned int j=0; j<width*height; j++) {
               unsigned int k=0;
               for (k=0; k<ncomp[i]; k++)
                  *dst++ = *src++;
               src += elemsize-k;
            }

            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
                            0, 0, width, height, GLformat[ncomp[i]],
                            GL_FLOAT, t);
            free(t);
         }
      }

   }

   CHECK_GL();
}
