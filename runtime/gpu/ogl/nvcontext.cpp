
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
                  GL_RGBA,
                  GL_RGB,
                  GL_RGBA };

static const unsigned int 
sizefactor[4] = { 1, 4, 3, 4 };


NVTexture::NVTexture ( size_t inWidth, 
                       size_t inHeight, 
                       GPUContext::TextureFormat inFormat) :
  OGLTexture(inWidth, inHeight, inFormat, 
             nvformats, nvtypes, sizefactor)
{
  _nativeFormat = nvformats[components()]; 
}


static const int nviAttribList[4][64] = {
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


NVContext::NVContext()
    : supportsFP40(false)
{}


NVContext *
NVContext::create() {
  NVContext *ctx = new NVContext();

  if (!ctx)
    return NULL;

  ctx->init(&nviAttribList, NULL, &nvpiAttribList);

  const char *ext = (const char *) glGetString(GL_EXTENSIONS);
  if(strstr(ext, "GL_NV_fragment_program2"))
      ctx->supportsFP40 = true;

  return ctx;
}


int 
NVContext::getShaderFormatRank (const char *name) const {
  if( strcmp(name, "arb") == 0 )
      return 1;
  if( strcmp(name, "fp40") == 0 )
      return 2;
  return -1;
}


GPUContext::TextureHandle 
NVContext::createTexture2D( unsigned int inWidth,
                            unsigned int inHeight, 
                            GPUContext::TextureFormat inFormat) {
  return (GPUContext::TextureHandle) 
    new NVTexture(inWidth, inHeight, inFormat);
}


static const char nvext[][64] = {
  "GL_ARB_fragment_program",
  "GL_NV_float_buffer",
  "GL_NV_fragment_program",
  "GL_NV_texture_rectangle",
  ""};

bool
NVContext::isCompatibleContext () {
  const char *ext = (const char *) glGetString(GL_EXTENSIONS);
  int p;

  for (p = 0; *nvext[p]; p++) {
    if (!strstr(ext, nvext[p]))
      return false;
  }
  return true;
}

bool
NVContext::isVendorContext () {
  const char *vendor = (const char *) glGetString(GL_VENDOR);
  return strstr(vendor, "NVIDIA") != NULL;
}

