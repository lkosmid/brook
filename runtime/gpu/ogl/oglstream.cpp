
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"
#include "ogltexture.hpp"
#include "oglwindow.hpp"
#include "oglfunc.hpp"
#include "oglcheckgl.hpp"

using namespace brook;

void 
OGLContext::copy_to_pbuffer(OGLTexture *texture) {
  int w = texture->width();
  int h = texture->height();

  wnd->bindPbuffer(texture->components());
  
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,  
                   (unsigned int) getPassthroughPixelShader());

  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture (GL_TEXTURE_RECTANGLE_NV, texture->id());

  glViewport (0, 0, w, h);
     
  if (w == 1 && h == 1) {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f, 0.5f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(3.0f, -1.0f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();
  } else if (h == 1) {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.5f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(w*2.0f, 0.5f);
    glVertex2f(3.0f, -1.0f);
    glTexCoord2f(0.0f, 0.5f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();
  } else if (w == 1) {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.5f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(0.5f, 0.0f);
    glVertex2f(3.0f, -1.0f);
    glTexCoord2f(0.5f, h*2.0f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();
  } else {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(w*2.0f, 0.0f);
    glVertex2f(3.0f, -1.0f);
    glTexCoord2f(0.0f, h*2.0f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();
  }
  glFinish();
  CHECK_GL();
}

void 
OGLContext::setTextureData(TextureHandle inTexture, 
                           const float* inData,
                           unsigned int inStrideBytes,
                           unsigned int inComponentCount ) {
  float *t;
  
  OGLTexture *oglTexture = (OGLTexture *) inTexture;
  
  bool fastPath = oglTexture->isFastSetPath( inStrideBytes, 
                                             inComponentCount);

  glBindTexture (GL_TEXTURE_RECTANGLE_NV, oglTexture->id());
 
  if (fastPath) {
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                    oglTexture->width(), 
                    oglTexture->height(), 
                    oglTexture->nativeFormat(),
                    GL_FLOAT, inData);
    return;
  }
  
  t = (float *) malloc (oglTexture->bytesize());
  oglTexture->copyToTextureFormat(inData, 
                                  inStrideBytes, 
                                  inComponentCount,
                                  t);

  glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                  oglTexture->width(), 
                  oglTexture->height(), 
                  oglTexture->nativeFormat(),
                  GL_FLOAT, t);
  
  free(t);
  CHECK_GL();
}

void 
OGLContext::getTextureData( TextureHandle inTexture,
                            float* outData,
                            unsigned int inStrideBytes,
                            unsigned int inComponentCount ) {
   float *t = outData;

   OGLTexture *oglTexture = (OGLTexture *) inTexture;
   
   bool fastPath = oglTexture->isFastGetPath( inStrideBytes, 
                                              inComponentCount); 
   if (!fastPath)
     t = (float *) malloc (oglTexture->bytesize());

   copy_to_pbuffer(oglTexture);
   CHECK_GL();
   
   glReadPixels (0, 0,
                 oglTexture->width(),
                 oglTexture->height(), 
                 oglTexture->nativeFormat(),
                 GL_FLOAT, t);
   CHECK_GL();

   if (!fastPath) {
     oglTexture->copyFromTextureFormat(t, 
                                       inStrideBytes, 
                                       inComponentCount,
                                       outData);
     free(t);
   }
  CHECK_GL();
}


void 
OGLContext::releaseTexture( TextureHandle inTexture ) {
  delete (OGLTexture *) inTexture;
}



