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
     glBindTexture (GL_TEXTURE_RECTANGLE_NV, id[i]);

     // Create a texture with NULL data
     glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, GLtype[ncomp[i]],
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
     GLReadData(cacheptr);
   }

  return cacheptr;
}

void
GLStream::releaseData (unsigned int flags) {

  if (flags == StreamInterface::WRITE ||
      flags == StreamInterface::READWRITE) {
     GLWriteData(cacheptr);
  }

  free (cacheptr);
}


void GLStream::Read(const void *p) {

  assert (p);
  GLWriteData(p);
}

void GLStream::Write(void *p) {

  assert (p);
  GLReadData(p);
}

GLStream::~GLStream () {

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
   if (size < 1024)
      std::cerr << size << " bytes\n";
   else if (size < 1024*1024)
      std::cerr << size/1024 << " KB\n";
   else
      std::cerr << size/(1024*1024) << " MB\n";

   return size;
}
