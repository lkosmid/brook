
#include "oglcontext"

static  
void copy_to_pbuffer(OGLTexture *texture) {
  
} 

bool
OGLContext::isFastSetPath( size_t inStrideBytes, 
                           size_t inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}

bool
OGLContext::isFastGetPath( size_t inStrideBytes, 
                           size_t inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}


void 
OGLContext::setTextureData(TextureHandle inTexture, 
                           const float* inData,
                           size_t inStrideBytes,
                           size_t  ) {
   OGLTexture *oglTexture = (OGLTexture *) inTexture;
   
   bool fastPath = oglTexture->isFastSetPath( inStrideBytes, 
                                              inComponentCount); 
   if (fastPath)
      t = inData;
   else {
      t = (float *) malloc (oglTexture->bytesize());
      oglTexture->copyToTextureFormat(inData, 
                                      inStrideBytes, 
                                      inComponentCount,
                                      t);
   }

   glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 
                   oglTexture->width(), 
                   oglTexture->height(), 
                   oglTexture->nativeFormat(),
                   GL_FLOAT, t);

   if (!fastPath) {
           free(t);
   }
}

void 
OGLContext::getTextureData( TextureHandle inTexture,
                            float* outData,
                            size_t inStrideBytes,
                            size_t inComponentCount ) {

   OGLTexture *oglTexture = (OGLTexture *) inTexture;
   
   bool fastPath = oglTexture->isFastGetPath( inStrideBytes, 
                                              inComponenetCount); 
   if (fastPath)
      t = outData;
   else 
      t = (float *) malloc (oglTexture->bytesize());

   copyToPbuffer(oglTexture);
   
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
