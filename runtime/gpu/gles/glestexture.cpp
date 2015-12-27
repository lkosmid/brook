#include <string.h>
#include "../gpucontext.hpp"

#include "glesfunc.hpp"
#include "glestexture.hpp"
#include "glescontext.hpp"
#include "glescheckgl.hpp"

using namespace brook;

// These are the official OpenGL formats. No vendor fully supports these yet for FBO's
static const unsigned int glFormatSTD[4][1] = {
                    {GL_LUMINANCE},
                    {GL_LUMINANCE_ALPHA},
                    {GL_RGB},
                    {GL_RGBA}};
static const unsigned int glTypeSTD[4][1] = {
                {GL_LUMINANCE},
                {GL_LUMINANCE_ALPHA},
                {GL_RGB},
                {GL_RGBA}};
static const unsigned int sizeFactorSTD[4][1] = { {1}, {2}, {3}, {4} };
static const unsigned int atomSizeSTD  [4][1] = { {1}, {1}, {1}, {1} };

#if 0

// These are specifically for ATI
// ATI currently cannot do single channel floats for FBO's, so we must use GL_RGB
// Also, there is a bug in render to FBO which fails to convert RGB to BGR so
// we invert the format to work around it (for now)
static const unsigned int glFormatATI[4][3] = {
                    {GL_BGR,GL_LUMINANCE,GL_LUMINANCE},
                    {GL_BGR,GL_LUMINANCE_ALPHA,GL_LUMINANCE_ALPHA},
                    {GL_BGR,GL_BGR,GL_BGR},
                    {GL_BGRA,GL_BGRA,GL_BGRA}};
static const unsigned int glTypeATI[4][3] = {
                {GL_RGB32F_ARB,            GL_LUMINANCE16,        GL_LUMINANCE},
                {GL_RGB32F_ARB,            GL_LUMINANCE16_ALPHA16,GL_LUMINANCE_ALPHA},
                {GL_RGB32F_ARB,            GL_RGB16,              GL_RGB},
                {GL_RGBA32F_ARB,           GL_RGBA16,             GL_RGBA}};
static const unsigned int sizeFactorATI[4][3] = { {3,1,1}, {3,2,2}, {3,3,3}, {4,4,4} };
static const unsigned int atomSizeATI  [4][3] = { {4,2,1}, {4,2,1}, {4,2,1}, {4,2,1} };

// These are specifically for NVidia
// NVidia only can do single and dual channel floats using a custom type
// Curiously, GL_FLOAT_RG32_NV won't combine with GL_LUMINANCE_ALPHA so while
// we store only two components in GPU memory, we must access via GL_RGB
static const unsigned int glFormatNV[4][3] = {
                    {GL_RED,GL_LUMINANCE,GL_LUMINANCE},
                    {GL_RGB,GL_LUMINANCE_ALPHA,GL_LUMINANCE_ALPHA},
                    {GL_RGB,GL_RGB,GL_RGB},
                    {GL_RGBA,GL_RGBA,GL_RGBA}};
static const unsigned int glTypeNV[4][3] = {
                {GL_FLOAT_R32_NV,          GL_LUMINANCE16,        GL_LUMINANCE},
                {GL_FLOAT_RG32_NV,         GL_LUMINANCE16_ALPHA16,GL_LUMINANCE_ALPHA},
                {GL_RGB32F_ARB,            GL_RGB16,              GL_RGB},
                {GL_RGBA32F_ARB,           GL_RGBA16,             GL_RGBA}};
static const unsigned int sizeFactorNV[4][3] = { {1,1,1}, {3,2,2}, {3,3,3}, {4,4,4} };
static const unsigned int atomSizeNV  [4][3] = { {4,2,1}, {4,2,1}, {4,2,1}, {4,2,1} };
#endif

/* Allocates a floating point texture */ 
GLESTexture::GLESTexture (GLESContext *ctx,
                        unsigned int width,
                        unsigned int height,
                        GPUContext::TextureFormat format):
   _width(width), _height(height), _format(format) {
   _elementType=GLES_FLOAT;
   
   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
   case GPUContext::kTextureFormat_Float2:
   case GPUContext::kTextureFormat_Float3:
   case GPUContext::kTextureFormat_Float4:
       _elementType=GLES_FLOAT;
       break;
   case GPUContext::kTextureFormat_Fixed1:
   case GPUContext::kTextureFormat_Fixed2:
   case GPUContext::kTextureFormat_Fixed3:
   case GPUContext::kTextureFormat_Fixed4:
       _elementType=GLES_FIXED;
       break;
   case GPUContext::kTextureFormat_ShortFixed1:
   case GPUContext::kTextureFormat_ShortFixed2:
   case GPUContext::kTextureFormat_ShortFixed3:
   case GPUContext::kTextureFormat_ShortFixed4:
       _elementType=GLES_SHORTFIXED;
       break;
   }
   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
   case GPUContext::kTextureFormat_Fixed1:
   case GPUContext::kTextureFormat_ShortFixed1:
      _components = 1;
      break;
   case GPUContext::kTextureFormat_Float2:
   case GPUContext::kTextureFormat_Fixed2:
   case GPUContext::kTextureFormat_ShortFixed2:
      _components = 2;
      break;
   case GPUContext::kTextureFormat_Float3:
   case GPUContext::kTextureFormat_ShortFixed3:
   case GPUContext::kTextureFormat_Fixed3:
      _components = 3;
      break;
   case GPUContext::kTextureFormat_Float4:
   case GPUContext::kTextureFormat_ShortFixed4:
   case GPUContext::kTextureFormat_Fixed4:
      _components = 4;
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
   /* { GLES_FLOAT, GLES_FIXED, GLES_SHORTFIXED }
   One component
   Two components
   Three components
   Four components
   */
   unsigned int glFormat  [4][1]; memcpy(glFormat,   glFormatSTD,   sizeof(glFormatSTD));
   unsigned int glType    [4][1]; memcpy(glType,     glTypeSTD,     sizeof(glTypeSTD));
   unsigned int sizeFactor[4][1]; memcpy(sizeFactor, sizeFactorSTD, sizeof(sizeFactorSTD));
   unsigned int atomSize  [4][1]; memcpy(atomSize,   atomSizeSTD,   sizeof(atomSizeSTD));
   _atomsize = atomSize[_components-1][_elementType];
   _bytesize = _width*_height*sizeFactor[_components-1][_elementType]*_atomsize;
   _elemsize = sizeFactor[_components-1][_elementType];
   _nativeFormat = glFormat[_components-1][_elementType];

   glGenTextures(1, &_id);
   //printf("Creating texture %u\n", _id);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture (GL_TEXTURE_2D, _id);
   CHECK_GL();
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glPixelStorei(GL_PACK_ALIGNMENT,1);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   CHECK_GL();
   // Create a texture with NULL data
   glTexImage2D (GL_TEXTURE_2D, 0, 
#ifdef GLES3
                 glType[_components-1][_elementType],
                 width, height, 0,
                 glFormat[_components-1][_elementType],
                 _elementType==GLES_FIXED?GL_UNSIGNED_BYTE:(_elementType==GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), 
#else
                 //For OpenGL ES 2.0 we don't have many options
                 GL_RGBA,
                 width, height, 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
#endif
                 NULL);
   CHECK_GL();
}

GLESTexture::~GLESTexture () {
  //printf("Deleting texture %u\n", _id);
  glDeleteTextures (1, &_id);
  CHECK_GL();
}


bool
GLESTexture::isFastSetPath( unsigned int inStrideBytes, 
                           unsigned int inWidth,
                           unsigned int inHeight,
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _elemsize*_atomsize &&
           inElemCount   == inWidth*inHeight);
}

bool
GLESTexture::isFastGetPath( unsigned int inStrideBytes, 
                           unsigned int inWidth,
                           unsigned int inHeight,
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _elemsize*_atomsize &&
           inElemCount   == inWidth*inHeight);
}


void
GLESTexture::copyToTextureFormat(const void *src, 
                                unsigned int srcStrideBytes, 
                                unsigned int srcElemCount,
                                void *dst) const {
   unsigned int i;
   
   switch (_components) {
   case 1:
   case 2:
   case 3:
   case 4:
      for (i=0; i<srcElemCount; i++) {
         memcpy(dst,src,_atomsize*_components);
         src = (((unsigned char *) (src)) + srcStrideBytes);
         dst = ((unsigned char *)dst) + _elemsize*_atomsize;
      }
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
}


void
GLESTexture::copyFromTextureFormat(const void *src, 
                                  unsigned int dstStrideBytes, 
                                  unsigned int dstElemCount,
                                  void *dst) const {
   unsigned int i;
   
   switch (_components) {
   case 1:
   case 2: 
   case 3:
   case 4:
      for (i=0; i<dstElemCount; i++) {
         if(1)//type==floating point
         {
            convert_fp_from_gpu(dst, src);
            dst = (((unsigned char *) (dst)) + dstStrideBytes);
            src = ((unsigned char *)src) + 4*_elemsize*_atomsize;
         }
         else
         {
            //In GLES 2.0 we read 4 components except in the case of char streams
            memcpy(dst,src,4*_atomsize*_components);
            dst = (((unsigned char *) (dst)) + dstStrideBytes);
            src = ((unsigned char *)src) + 4*_elemsize*_atomsize;
         }
      }
      break;
   default: 
      GPUError("Unknown Texture Format");
   }
}

void 
GLESTexture::convert_fp_from_gpu(void *dst, const void *src) const
{
   const unsigned char* _src= (unsigned char*) src;
   __float_t_ f;

   assert(sizeof(float) == sizeof(f));
   memset(&f, 0, sizeof(f));

   f.exp=_src[0];
   f.mant = ((_src[1] & 0x7F) << 16) ;
   f.sign= ((_src[1] & 0x80)!=0) ;
   f.mant = (f.mant & 0x7F0000) | (_src[2] << 8) ;
   f.mant = (f.mant & 0x7FFF00) | _src[3] ;

   memcpy(dst,&f,sizeof(f));
//   *((float*)dst)=f.f;
}

void 
GLESTexture::convert_fp_to_gpu(void *dst, const void *src) const
{
   unsigned char* _dst= (unsigned char*) dst;
   __float_t_ f;

   f.f= *((float*) src);

   _dst[0] = f.exp;
   _dst[1] = (f.sign<<7) | (f.mant >> 16);
   _dst[2] = (f.mant >> 8) & 0x00FF;
   _dst[3] = f.mant & 0x00FF;
}

void GLESTexture::getRectToCopy(
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

void GLESTexture::setATData(
  const void* inStreamData, unsigned int inStrideBytes, unsigned int inRank,
  const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
  void* ioTextureData )
{
  // TIM: get all the fun information out of our streams
  size_t r;
  size_t rank = inRank;
  size_t domainMin[4] = {0,0,0,0};
  size_t domainMax[4] = {1,1,1,1};
  size_t strides[4] = {0,0,0,0};
  size_t stride = 1;
  const void *streamElement=inStreamData;
  for( r = 0; r < rank; r++ )
  {
    size_t d = rank - (r+1);

    domainMin[r] = inDomainMin[d];
    domainMax[r] = inDomainMax[d];
    size_t streamExtent = inExtents[d];
    strides[r] = stride;
    stride *= streamExtent;
  }
  
  
  const size_t componentSize = _atomsize*_elemsize;
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

          void* textureElement = ((unsigned char*)ioTextureData) + streamIndex*_components*_atomsize;
          memcpy(textureElement,streamElement,componentSize);
          streamElement=((unsigned char *)streamElement)+componentSize;
          
        }
      }
    }
  }
}

void GLESTexture::getATData(
  void* outStreamData, unsigned int inStrideBytes, unsigned int inRank,
  const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
  const void* inTextureData )
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

  const size_t streamElementSize = inStrideBytes;
  const size_t copySize =  _atomsize*_elemsize;
  void* streamElement = outStreamData;
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
          const void* textureElement = ((unsigned char*)inTextureData) + streamIndex*copySize;
assert(0);
          //In GLES 2.0 we read 4 components except in the case of char streams
          memcpy(streamElement,textureElement,4*copySize);
          streamElement=((unsigned char*)streamElement)+streamElementSize;
        }
      }
    }
  }
}
