

#include "oglcontext"


OGLTexture::OGLTexture (size_t width,
                        size_t height,
                        TextureFormat format):
   _width(width), _height(height), _format(format) {

   switch (_format) {
   case kTextureFormat_Float1:
      _components = 1;
      break;
   case kTextureFormat_Float2:
      _components = 2;
      break;
   case kTextureFormat_Float3:
      _components = 3;
      break;
   case kTextureFormat_Float4:
      _components = 4;
      break;
   default: 
      GPUError("Unkown Texture Format");
   }

   _bytesize = _width*_height*_components*sizeof(float);
}

void
OGLTexture::copyToTextureFormat(const float *src, 
                                size_t srcStrideBytes, 
                                size_t srcElemCount,
                                float *dst) {
   size_t i;
   
   switch (_format) {
   case kTextureFormat_Float1:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case kTextureFormat_Float2:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case kTextureFormat_Float3:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         *dst++ = *(src+2);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case kTextureFormat_Float4:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         *dst++ = *(src+2);
         *dst++ = *(src+3);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
}


void
OGLTexture::copyFromTextureFormat(const float *src, 
                                  size_t dstStrideBytes, 
                                  size_t dstElemCount,
                                  float *dst) {
   size_t i;
   
   switch (_format) {
   case kTextureFormat_Float1:
      for (i=0; i<dstElemCount; i++) {
         *dst = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case kTextureFormat_Float2:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case kTextureFormat_Float3:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         *(dst+2) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case kTextureFormat_Float4:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         *(dst+2) = *src++;
         *(dst+3) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
}

