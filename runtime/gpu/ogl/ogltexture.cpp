
#include "../gpucontext.hpp"

#include "oglfunc.hpp"
#include "ogltexture.hpp"
#include "oglcheckgl.hpp"

using namespace brook;

/* Allocates a floating point texture */ 
OGLTexture::OGLTexture (unsigned int width,
                        unsigned int height,
                        GPUContext::TextureFormat format,
                        const unsigned int glFormat[4],
                        const unsigned int glType[4],
                        const unsigned int sizeFactor[4]):
   _width(width), _height(height), _format(format) {

   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
      _components = 1;
      break;
   case GPUContext::kTextureFormat_Float2:
      _components = 2;
      break;
   case GPUContext::kTextureFormat_Float3:
      _components = 3;
      break;
   case GPUContext::kTextureFormat_Float4:
      _components = 4;
      break;
   default: 
      GPUError("Unkown Texture Format");
   }

   _bytesize = _width*_height*sizeFactor[_components-1]*sizeof(float);
   _elemsize = sizeFactor[_components-1];
   _nativeFormat = glFormat[_components-1];

   glGenTextures(1, &_id);
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, _id);
   CHECK_GL();
 
   // Create a texture with NULL data
   glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 
                 glType[_components-1],
                 width, height, 0,
                 glFormat[_components-1],
                 GL_FLOAT, NULL);
   CHECK_GL();
   
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   CHECK_GL();
}

OGLTexture::~OGLTexture () {
  glDeleteTextures (1, &_id);
  CHECK_GL();
}


bool
OGLTexture::isFastSetPath( unsigned int inStrideBytes, 
                           unsigned int inWidth,
                           unsigned int inHeight,
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _elemsize*sizeof(float) &&
           inElemCount   == inWidth*inHeight);
}

bool
OGLTexture::isFastGetPath( unsigned int inStrideBytes, 
                           unsigned int inWidth,
                           unsigned int inHeight,
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _elemsize*sizeof(float) &&
           inElemCount   == inWidth*inHeight);
}


void
OGLTexture::copyToTextureFormat(const float *src, 
                                unsigned int srcStrideBytes, 
                                unsigned int srcElemCount,
                                float *dst) const {
   unsigned int i;
   
   switch (_components) {
   case 1:
      for (i=0; i<srcElemCount; i++) {
         *dst = *src;
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
         dst += _elemsize;
      }
      break;
   case 2:
      for (i=0; i<srcElemCount; i++) {
         *dst = *src;
         *(dst+1) = *(src+1);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
         dst += _elemsize;
      }
      break;
   case 3:
      for (i=0; i<srcElemCount; i++) {
         *dst = *src;
         *(dst+1) = *(src+1);
         *(dst+2) = *(src+2);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
         dst += _elemsize;
      }
      break;
   case 4:
      for (i=0; i<srcElemCount; i++) {
         *dst = *src;
         *(dst+1) = *(src+1);
         *(dst+2) = *(src+2);
         *(dst+3) = *(src+3);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
         dst += _elemsize;
      }
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
}


void
OGLTexture::copyFromTextureFormat(const float *src, 
                                  unsigned int dstStrideBytes, 
                                  unsigned int dstElemCount,
                                  float *dst) const {
   unsigned int i;
   
   switch (_components) {
   case 1:
      for (i=0; i<dstElemCount; i++) {
         *dst = *src;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
         src += _elemsize;
      }
      break;
   case 2:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src;
         *(dst+1) = *(src+1);
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
         src += _elemsize;
      }
      break;
   case 3:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src;
         *(dst+1) = *(src+1);
         *(dst+2) = *(src+2);
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
         src += _elemsize;
      }
      break;
   case 4:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src;
         *(dst+1) = *(src+1);
         *(dst+2) = *(src+2);
         *(dst+3) = *(src+3);
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
         src += _elemsize;
      }
      break;
   default: 
      GPUError("Unknown Texture Format");
   }
}

void OGLTexture::getRectToCopy(
  unsigned int inRank, const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
  int& outMinX, int& outMinY, int& outMaxX, int& outMaxY, size_t& outBaseOffset, bool& outFullStream, bool inUsesAddressTranslation )
{
  size_t r;
  size_t rank = inRank;
  size_t domainMin[4] = {0,0,0,0};
  size_t domainMax[4] = {1,1,1,1};
  size_t domainExtents[4] = {0,0,0,0};
  size_t minIndex = 0;
  size_t maxIndex = 0;
  size_t stride = 1;

  bool fullStream = true;
  for( r = 0; r < rank; r++ )
  {
    size_t d = rank - (r+1);

    domainMin[r] = inDomainMin[d];
    domainMax[r] = inDomainMax[d];
    domainExtents[r] = domainMax[r] - domainMin[r];
    size_t streamExtent = inExtents[d];
    if( streamExtent != domainExtents[r] )
      fullStream = false;

    minIndex += domainMin[r]*stride;
    maxIndex += (domainMax[r]-1)*stride;
    stride *= streamExtent;
  }

  if( inUsesAddressTranslation )
  {
    size_t minX = minIndex % _width;
    size_t minY = minIndex / _width;
    size_t maxX = maxIndex % _width;
    size_t maxY = maxIndex / _width;

    size_t baseOffset = minX;

    if( minY != maxY )
    {
      minX = 0;
      maxX = _width-1;
    }

    outMinX = minX;
    outMinY = minY;
    outMaxX = maxX+1;
    outMaxY = maxY+1;
    outBaseOffset = baseOffset;
  }
  else
  {
    outMinX = domainMin[0];
    outMinY = domainMin[1];
    outMaxX = domainMax[0];
    outMaxY = domainMax[1];
    outBaseOffset = 0;
  }
  outFullStream = fullStream;
}

void OGLTexture::setATData(
  const float* inStreamData, unsigned int inStrideBytes, unsigned int inRank,
  const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
  float* ioTextureData )
{
  // TIM: get all the fun information out of our streams
  size_t r;
  size_t rank = inRank;
  size_t domainMin[4] = {0,0,0,0};
  size_t domainMax[4] = {1,1,1,1};
  size_t strides[4] = {0,0,0,0};
  size_t stride = 1;

  for( r = 0; r < rank; r++ )
  {
    size_t d = rank - (r+1);

    domainMin[r] = inDomainMin[d];
    domainMax[r] = inDomainMax[d];
    size_t streamExtent = inExtents[d];
    strides[r] = stride;
    stride *= streamExtent;
  }

  size_t streamComponents = inStrideBytes / sizeof(float);

  const float* streamElement = inStreamData;
  size_t x, y, z, w;
  for( w = domainMin[3]; w < domainMax[3]; w++ )
  {
    size_t offsetW = w*strides[3];
    for( z = domainMin[2]; z < domainMax[2]; z++ )
    {
      size_t offsetZ = offsetW + z*strides[2];
      for( y = domainMin[1]; y < domainMax[1]; y++ )
      {
        size_t offsetY = offsetZ + y*strides[1];
        for( x = domainMin[0]; x < domainMax[0]; x++ )
        {
          size_t streamIndex = offsetY + x*strides[0];

          float* textureElement = ioTextureData + streamIndex*_elemsize;

          for( size_t c = 0; c < streamComponents; c++ )
            *textureElement++ = *streamElement++;
        }
      }
    }
  }
}

void OGLTexture::getATData(
  float* outStreamData, unsigned int inStrideBytes, unsigned int inRank,
  const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
  const float* inTextureData )
{
  // TIM: get all the fun information out of our streams
  size_t r;
  size_t rank = inRank;
  size_t domainMin[4] = {0,0,0,0};
  size_t domainMax[4] = {1,1,1,1};
  size_t strides[4] = {0,0,0,0};
  size_t stride = 1;

  for( r = 0; r < rank; r++ )
  {
    size_t d = rank - (r+1);

    domainMin[r] = inDomainMin[d];
    domainMax[r] = inDomainMax[d];
    size_t streamExtent = inExtents[d];
    strides[r] = stride;
    stride *= streamExtent;
  }

  size_t streamComponents = inStrideBytes / sizeof(float);

  float* streamElement = outStreamData;
  size_t x, y, z, w;
  for( w = domainMin[3]; w < domainMax[3]; w++ )
  {
    size_t offsetW = w*strides[3];
    for( z = domainMin[2]; z < domainMax[2]; z++ )
    {
      size_t offsetZ = offsetW + z*strides[2];
      for( y = domainMin[1]; y < domainMax[1]; y++ )
      {
        size_t offsetY = offsetZ + y*strides[1];
        for( x = domainMin[0]; x < domainMax[0]; x++ )
        {
          size_t streamIndex = offsetY + x*strides[0];

          const float* textureElement = inTextureData + streamIndex*_elemsize;

          for( size_t c = 0; c < streamComponents; c++ )
            *streamElement++ = *textureElement++;
        }
      }
    }
  }
}
