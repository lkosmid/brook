#include <stdio.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLStream::NV30GLStream (NV30GLRunTime *rt,
                            __BRTStreamType type, int dims, int extents[])
  : Stream(type), runtime(rt) { 
  
  int i;

  // Initialize width and height
  height  = 1;
  switch (dims) {
  case 2:
    height = extents[1];
  case 1:
    width = extents[0];
    break;
  default:
    fprintf (stderr, 
             "NV30GL Backend only supports 1D and 2D streams");
    exit(1);
  }
  
  // Initialize ncomp
  switch (type) {
  case __BRTFLOAT:
    ncomp = 1;
    break;
  case __BRTFLOAT2:
    ncomp = 2;
    break;
  case __BRTFLOAT3:
    ncomp = 3;
    break;
  case __BRTFLOAT4:
    ncomp = 4;
    break;
  default:
    fprintf (stderr, "NV30GL: Unsupported stream type created\n");
    exit(1);
  }
  
  glGenTextures(1, &id);
  glActiveTextureARB(GL_TEXTURE0_ARB+15);
  glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
  
  // Create a texture with NULL data
  glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, GLtype[ncomp], 
                width, height, 0, 
                GLformat[ncomp], GL_FLOAT, NULL);
  CHECK_GL();
  
  // Initialize the cacheptr
  cacheptr = malloc (sizeof(float)*ncomp*width*height);
  
  // Initialize extents
  for (i=0; i<dims; i++)
    this->extents[i] = extents[i];
  this->dims = dims;
}


void * 
NV30GLStream::getData (unsigned int flags) {

  if (flags == StreamInterface::READ ||
      flags == StreamInterface::READWRITE) {
    
    glActiveTextureARB(GL_TEXTURE0_ARB+15);
    glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
    glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp],
                  GL_FLOAT, cacheptr);
    
    CHECK_GL();
  }

  return cacheptr;
}

void 
NV30GLStream::releaseData (unsigned int flags) {
  
  if (flags == StreamInterface::WRITE ||
      flags == StreamInterface::READWRITE) {
  
    glActiveTextureARB(GL_TEXTURE0_ARB+15);
    glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);

    glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                    0, 0, width, height, GLformat[ncomp],
                    GL_FLOAT, cacheptr);
    CHECK_GL();
  }
}


void NV30GLStream::Read(const void *p) {
    
  assert (p);
  
  glActiveTextureARB(GL_TEXTURE0_ARB+15);
  glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
  glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                  0, 0, width, height, GLformat[ncomp],
                  GL_FLOAT, p);
  CHECK_GL();
}

void NV30GLStream::Write(void *p) {
  
  assert (p);
  
  glActiveTextureARB(GL_TEXTURE0_ARB+15);
  glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
  
  glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp],
                GL_FLOAT, p);
  
  CHECK_GL();
}

NV30GLStream::~NV30GLStream () {
  free (cacheptr);
  glDeleteTextures (1, &id);
  CHECK_GL();
}
