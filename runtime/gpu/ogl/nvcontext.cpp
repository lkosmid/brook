
#include <nvcontext>

static const int NVTexture::nvtypes[] =   {GL_FLOAT_R32_NV,
                                           GL_FLOAT_RG32_NV,
                                           GL_FLOAT_RGB32_NV,
                                           GL_FLOAT_RGBA32_NV};

static const int NVTexture::sizefactor[] = {1, 2, 3, 4};

NVTexture::NVTexture ( size_t inWidth, 
                       size_t inHeight, 
                       TextureFormat inFormat) :
  OGLTexture(inWidth, inHeight, inFormat, 
             nvformats, nvtypes, sizefactor),
  _nativeFormat(nvformats[this->components()-1])
{ }



TextureHandle 
NVTexture::createTexture2D( unsigned int inWidth, 
                            unsigned int inHeight, 
                            TextureFormat inFormat) {
  return (TextureHandle) new NVTexture(inWidth, inHeight, inFormat);
}
