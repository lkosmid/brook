
#include "aticontext.hpp"

#include "oglfunc.hpp"

#ifdef WIN32

#ifndef WGL_ATI_pixel_format_float
#define WGL_TYPE_RGBA_FLOAT_ATI             0x21A0
#define GL_TYPE_RGBA_FLOAT_ATI              0x8820
#endif

#ifndef WGL_ARB_pixel_format
#define WGL_PIXEL_TYPE_ARB             0x2013
#endif

#else

#ifndef GLX_RGBA_FLOAT_ATI_BIT
#define GLX_RGBA_FLOAT_ATI_BIT          0x0100
#endif

#endif


#ifndef GL_ATI_texture_float
#define GL_RGBA_FLOAT32_ATI               0x8814
#define GL_RGB_FLOAT32_ATI                0x8815
#define GL_ALPHA_FLOAT32_ATI              0x8816
#define GL_INTENSITY_FLOAT32_ATI          0x8817
#define GL_LUMINANCE_FLOAT32_ATI          0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_ATI    0x8819
#endif

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
#ifdef WIN32
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 },
  { WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI, 0, 0 }
#else
  {  GLX_RGBA_FLOAT_ATI_BIT, GL_TRUE,  0,0},
  {  GLX_RGBA_FLOAT_ATI_BIT, GL_TRUE,  0,0},
  {  GLX_RGBA_FLOAT_ATI_BIT, GL_TRUE,  0,0},
  {  GLX_RGBA_FLOAT_ATI_BIT, GL_TRUE,  0,0}
#endif
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

