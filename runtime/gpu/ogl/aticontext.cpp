
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "aticontext.hpp"

#include "wglATI.h"
//#include "glATI.h"

#include "glext.h"
#include "wglext.h"


using namespace brook;

static const unsigned int 
atitypes[4] =   {GL_LUMINANCE_FLOAT32_ATI,
                 GL_RGBA_FLOAT32_ATI,
                 GL_RGB_FLOAT32_ATI,
                 GL_RGBA_FLOAT32_ATI};

static const unsigned int 
atiformats[4] =  {GL_RED,
                  GL_RGBA,
                  GL_RGB,
                  GL_RGBA};

static const unsigned int 
sizefactor[4] = { 1, 4, 3, 4 };

ATITexture::ATITexture ( size_t inWidth, 
                       size_t inHeight, 
                       GPUContext::TextureFormat inFormat) :
  OGLTexture(inWidth, inHeight, inFormat, 
             atiformats, atitypes, sizefactor),
  _nativeFormat(atiformats[components()]) 
{
}

static const int atiiAttribList[4][64] = {
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 }
};


ATIContext::ATIContext() {}

ATIContext *
ATIContext::create() {
  ATIContext *ctx = new ATIContext();

  if (!ctx)
    return NULL;

  ctx->init(&atiiAttribList, NULL, NULL);

  return ctx;
}


int 
ATIContext::getShaderFormatRank (const char *name) const {
  if( strcmp(name, "arb") == 0 )
      return 1;
  return -1;
}


GPUContext::TextureHandle 
ATIContext::createTexture2D( unsigned int inWidth,
                            unsigned int inHeight, 
                            GPUContext::TextureFormat inFormat) {
  return (GPUContext::TextureHandle) 
    new ATITexture(inWidth, inHeight, inFormat);
}


static const char atiext[][64] = {
  "GL_ARB_fragment_program",
  "GL_ARB_vertex_program",
  "GL_ATI_draw_buffers",
  "GL_EXT_texture_rectangle",
  "GL_ATI_texture_float",
  ""};


bool
ATIContext::isCompatibleContext () {
  const char *ext = (const char *) glGetString(GL_EXTENSIONS);
  int p;

  for (p = 0; *atiext[p]; p++) {
    if (!strstr(ext, atiext[p]))
      return false;
  }
  return true;
}

bool
ATIContext::isVendorContext () {
  const char *vendor = (const char *) glGetString(GL_VENDOR);
  return strstr(vendor, "ATI") != NULL;
}

