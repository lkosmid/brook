
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"
#include "ogltexture.hpp"

using namespace brook;

static  
void copy_to_pbuffer(OGLTexture *texture) {
  GPUError("to be implemented...");
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
   
   glReadPixels (0, 0,
                 oglTexture->width(),
                 oglTexture->height(), 
                 oglTexture->format(),
                 GL_FLOAT, t);

   if (!fastPath) {
     oglTexture->copyFromTextureFormat(t, 
                                       inStrideBytes, 
                                       inComponentCount,
                                       outData);
     free(t);
   }
}


void 
OGLContext::releaseTexture( TextureHandle inTexture ) {
  delete (OGLTexture *) inTexture;
}



