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
     width = extents[1];
     height = extents[0];
     break;
  case 1:
     width = extents[0];
     break;
  default:
     fprintf (stderr, 
              "NV30GL Backend only supports 1D and 2D streams");
     exit(1);
  }
  
  // Initialize ncomp
  ncomp = type;
  if (ncomp < 1 || ncomp > 4) {
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
  cacheptr = malloc (sizeof(float)*4*width*height);
  
  // Initialize extents
  for (i=0; i<dims; i++)
    this->extents[i] = extents[i];
  this->dims = dims;
}


void
NV30GLStream::GLReadData (void *data) {

   glActiveTextureARB(GL_TEXTURE0_ARB+15);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
   
   if (ncomp == 2) {
      float4 *p = (float4 *) malloc (sizeof(float)*4*width*height);
      glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA,
                    GL_FLOAT, p);
       
       for (unsigned int i=0; i<width*height; i++) {
          ((float2 *) data)[i].x = p[i].x;
          ((float2 *) data)[i].y = p[i].y;
       }
       
       free (p);
    } else {    
       glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp],
                     GL_FLOAT, data);
    }
    CHECK_GL();
}

void
NV30GLStream::GLWriteData (const void *data) {
   
   glActiveTextureARB(GL_TEXTURE0_ARB+15);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
   
    if (ncomp == 2) {
       float4 *p = (float4 *) malloc (sizeof(float)*4*width*height);
       for (unsigned int i=0; i<width*height; i++) {
          p[i].x = ((float2 *) data)[i].x;
          p[i].y = ((float2 *) data)[i].y;
          p[i].z = 0.0f;
          p[i].w = 0.0f;
       }
       
       glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                       0, 0, width, height, GL_RGBA,
                       GL_FLOAT, p);
       
       free (p);
    } else {    
       glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
                       0, 0, width, height, GLformat[ncomp],
                       GL_FLOAT, data);
    }
    
    CHECK_GL();
}

void * 
NV30GLStream::getData (unsigned int flags) {

  if (flags == StreamInterface::READ ||
      flags == StreamInterface::READWRITE) {    
     GLReadData(cacheptr);
   }

  return cacheptr;
}

void 
NV30GLStream::releaseData (unsigned int flags) {
  
  if (flags == StreamInterface::WRITE ||
      flags == StreamInterface::READWRITE) {
     GLWriteData(cacheptr);
  }
}


void NV30GLStream::Read(const void *p) {
    
  assert (p);
  GLWriteData(p);
}

void NV30GLStream::Write(void *p) {
  
  assert (p);
  GLReadData(p);
}

NV30GLStream::~NV30GLStream () {
  free (cacheptr);
  glDeleteTextures (1, &id);
  CHECK_GL();
}
