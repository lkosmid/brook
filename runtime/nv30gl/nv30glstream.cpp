#include <stdio.h>
#include <stdlib.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLStream::NV30GLStream (NV30GLRunTime *rt,
                            __BRTStreamType type, int dims, 
                            const int extents[])
  : elementType(type), runtime(rt) { 

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

  if (width > NV30GLRunTime::workspace
      || height > NV30GLRunTime::workspace) {

    fprintf (stderr, 
             "NV30GL: Cannot create streams larger than: %d %d\n",
             NV30GLRunTime::workspace, NV30GLRunTime::workspace);
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
  cacheptr = NULL;
  
  // Initialize extents
  for (i=0; i<dims; i++)
    this->extents[i] = extents[i];
  this->dims = dims;
  
  next = NULL;
  prev = NULL;
  if (rt->streamlist == NULL)
     rt->streamlist = this;
  else {
     rt->streamlist->prev = this;
     next = rt->streamlist;
     rt->streamlist = this;
  }
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
       
#if 1
       if (width == 1 && height == 1) {
          glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp],
                        GL_FLOAT, data);
          return;
       }

       glActiveTextureARB(GL_TEXTURE0_ARB);
       glBindTexture (GL_TEXTURE_RECTANGLE_NV, id);
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
       glReadPixels (0, 0, width, height, GLformat[ncomp],
                     GL_FLOAT, data);
#else
       glFinish();
       glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, GLformat[ncomp],
                     GL_FLOAT, data);
#endif
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
  
  cacheptr = malloc (sizeof(float)*4*width*height);
  
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

  free (cacheptr);
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
  glDeleteTextures (1, &id);
  CHECK_GL();

  if (prev)
     prev->next = next;
  if (next)
     next->prev = prev;
  if (runtime->streamlist == this)
     runtime->streamlist = next;
}
