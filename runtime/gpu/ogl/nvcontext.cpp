
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"
#include "wglext.h"

#include "nvcontext.hpp"

using namespace brook;

static const unsigned int 
nvtypes[4] =   {GL_FLOAT_R32_NV,
               GL_FLOAT_RG32_NV,
               GL_FLOAT_RGB32_NV,
               GL_FLOAT_RGBA32_NV};

static const unsigned int 
nvformats[4] =  { GL_RED,
                 GL_LUMINANCE_ALPHA,
                 GL_RGB,
                 GL_RGBA };

static const unsigned int 
sizefactor[4] = { 1, 2, 3, 4 };


NVTexture::NVTexture ( size_t inWidth, 
                       size_t inHeight, 
                       GPUContext::TextureFormat inFormat) :
  OGLTexture(inWidth, inHeight, inFormat, 
             nvtypes, nvformats, sizefactor),
  _nativeFormat(nvformats[components()]) 
{

}

bool 
NVContext::isValidShaderNameString (const char *name) const {
  return strcmp(name, "arb") == 0;
}


GPUContext::TextureHandle 
NVContext::createTexture2D( unsigned int inWidth,
                            unsigned int inHeight, 
                            GPUContext::TextureFormat inFormat) {
  return (GPUContext::TextureHandle) 
    new NVTexture(inWidth, inHeight, inFormat);
}


static const int nviAttribList[4][16] = {
  {  WGL_FLOAT_COMPONENTS_NV,                     GL_TRUE, 
     WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV,    GL_TRUE,
     0,0},
  {  WGL_FLOAT_COMPONENTS_NV,                     GL_TRUE, 
     WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV,   GL_TRUE,
     0,0},
  {  WGL_FLOAT_COMPONENTS_NV,                     GL_TRUE, 
     WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV,  GL_TRUE,
     0,0},
  {  WGL_FLOAT_COMPONENTS_NV,                     GL_TRUE, 
     WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV, GL_TRUE,
     0,0}
};


static const int nvpiAttribList[4][16] = {
  {  WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_R_NV,
     WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
     0,0},
  {  WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RG_NV,
     WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
     0,0},
  {  WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RGB_NV,
     WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
     0,0},
  {  WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RGBA_NV,
     WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
     0,0}
};

void 
NVContext::getVendorAttribs(const int   (**iAttrib)[4][16],
                            const float (**fAttrib)[4][16],
                            const int   (**piAttrib)[4][16]) {

  *iAttrib = &nviAttribList;
  *fAttrib = NULL;
  *piAttrib = &nvpiAttribList;
}

