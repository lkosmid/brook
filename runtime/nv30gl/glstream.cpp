#include <stdlib.h>
#include <iostream>

#include "glruntime.hpp"

using namespace brook;

/*
 * I don't know if this is really NV specific or not.  This code may have
 * to move to the subclasses once I get the ARB stuff done. --Jeremy.
 */
const GLenum GLtype[] = {
   0, GL_FLOAT_R32_NV, GL_FLOAT_RG32_NV, GL_FLOAT_RGB32_NV, GL_FLOAT_RGBA32_NV
};

GLStream::GLStream (GLRunTime *rt, int fieldCount,
                    const __BRTStreamType fieldType[],
                    int dims, const int extents[])
  : runtime(rt)
{
   int i;

  // Initialize width and height
  height  = 1;
  switch (dims) {
  case 2:
     width = extents[1];
     height = extents[0];
     break;
  case 1:
     width = extents[0];
     break;
  default:
     std::cerr << "GL Backend only supports 1D and 2D streams";
     exit(1);
  }

  if (width > GLRunTime::workspace
      || height > GLRunTime::workspace) {

     std::cerr << "GL: Cannot create streams larger than: "
               << GLRunTime::workspace << " " << GLRunTime::workspace << "\n";
    exit(1);
  }

  id = (GLuint *) malloc (fieldCount * sizeof (GLuint));
  ncomp = (unsigned int *) malloc (fieldCount * sizeof(unsigned int));
  stride = (unsigned int *) malloc (fieldCount * sizeof(unsigned int));
  nfields = (unsigned int) fieldCount;

  glGenTextures(nfields, id);
  CHECK_GL();

  elemsize = 0;
  for (i=0; i<fieldCount; i++) {
     // Initialize ncomp
     ncomp[i] = fieldType[i];
     if (ncomp[i] < 1 || ncomp[i] > 4) {
        std::cerr << "GL: Unsupported stream type created\n";
        exit(1);
     }

     elemsize += ncomp[i];

     if (!i)
        stride[i] = 0;
     else
        stride[i] = stride[i-1]+ncomp[i-1];

     glActiveTextureARB(GL_TEXTURE0_ARB+15);
     glBindTexture (GL_TEXTURE_RECTANGLE_EXT, id[i]);

     // Create a texture with NULL data
     glTexImage2D (GL_TEXTURE_RECTANGLE_EXT, 0, GLtype[ncomp[i]],
                   width, height, 0,
                   GLformat[ncomp[i]], GL_FLOAT, NULL);
     CHECK_GL();
  }

  // Initialize the cacheptr
  cacheptr = NULL;

  // Initialize extents
  for (i=0; i<dims; i++)
    this->extents[i] = extents[i];
  this->dims = dims;

  // Add to stream alloc linked list
  prev = NULL;
  next = rt->streamlist;
  if (next) next->prev = this;
  rt->streamlist = this;
}


void *
GLStream::getData (unsigned int flags) {

   if (nfields == 1)
      cacheptr = malloc (sizeof(float)*4*width*height);
   else {
      int size = 0;
      for (unsigned int i=0; i<nfields; i++)
         size += ncomp[i];
      cacheptr = malloc (sizeof(float)*size*width*height);
   }

  if (flags == StreamInterface::READ ||
      flags == StreamInterface::READWRITE) {
     Write(cacheptr);
   }

  return cacheptr;
}

void
GLStream::releaseData (unsigned int flags) {

  if (flags == StreamInterface::WRITE ||
      flags == StreamInterface::READWRITE) {
     Read(cacheptr);
  }

  free (cacheptr);
}


void
GLStream::Write(void *data)
{
   float *p = (float *) data;
   unsigned int i;

   assert(data);
   for (i=0; i<nfields; i++) {

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_RECTANGLE_EXT, id[i]);

      if (ncomp[i] == 2) {
         float4 *t = (float4 *) malloc (sizeof(float)*4*width*height);
         glGetTexImage(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA,
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
            glGetTexImage(GL_TEXTURE_RECTANGLE_EXT, 0, GLformat[ncomp[i]],
                          GL_FLOAT, (void *)(t));
            return;
         }
#if 1

         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, runtime->passthrough_id);

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
         glGetTexImage(GL_TEXTURE_RECTANGLE_EXT, 0, GLformat[ncomp[i]],
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
GLStream::Read(const void *data)
{
   unsigned int i;
   float *p = (float*) data;

   assert(data);
   glActiveTextureARB(GL_TEXTURE0_ARB);

   for (i=0; i<nfields; i++) {

      glBindTexture (GL_TEXTURE_RECTANGLE_EXT, id[i]);

      if (ncomp[i] == 2) {
         float4 *t = (float4 *) malloc (sizeof(float)*4*width*height);
         for (unsigned int j=0; j<width*height; j++) {
            t[j].x = ((float2 *) (p+stride[i]))[j].x;
            t[j].y = ((float2 *) (p+stride[i]))[j].y;
            t[j].z = 0.0f;
            t[j].w = 0.0f;
         }

         glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                         0, 0, width, height, GL_RGBA, GL_FLOAT, t);

         free (t);
      } else {

         if (nfields == 1)
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
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

            glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                            0, 0, width, height, GLformat[ncomp[i]],
                            GL_FLOAT, t);
            free(t);
         }
      }

   }

   CHECK_GL();
}


GLStream::~GLStream ()
{
   glDeleteTextures (nfields, id);
   CHECK_GL();

   free (ncomp);
   free (stride);
   free (id);

   if (prev)
      prev->next = next;
   if (next)
      next->prev = prev;
   if (runtime->streamlist == this)
      runtime->streamlist = next;
}

int
GLStream::printMemUsage(void)
{
   int size;
   int totncomp = 0;

   for (unsigned int i=0; i< nfields; i++)
      totncomp += ncomp[i];

   size = width*height*totncomp*sizeof(float);

   std::cerr << "Stream    (" << width << " x "
             << height << " x " << totncomp
             << "): ";
   if (size < 1024) {
      std::cerr << size << " bytes\n";
   } else if (size < 1024*1024) {
      std::cerr << size/1024 << " KB\n";
   } else {
      std::cerr << size/(1024*1024) << " MB\n";
   }

   return size;
}
