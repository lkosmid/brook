
#include "oglfunc.hpp"
#include "oglcontext.hpp"
#include "oglwindow.hpp"
#include "ogltexture.hpp"
#include "oglwritequery.hpp"
#include "oglcheckgl.hpp"

using namespace brook;

OGLContext *
OGLContext::create(const char* device) {
  OGLContext *ctx = new OGLContext();

  if (!ctx)
    return NULL;

  ctx->init(device);

  return ctx;
}

void
OGLContext::init (const char* device) {
  int image_units;
  int tex_coords;
  
  _wnd = new OGLWindow(device);

  _wnd->initFBO();

  _passthroughVertexShader = NULL;
  _passthroughPixelShader = NULL;

  const char *vendor = (const char *) glGetString(GL_VENDOR);
  _isATI=(strstr(vendor, "ATI") != NULL);
  _isNVidia=(strstr(vendor, "NVIDIA") != NULL);
  _havePBOs=!!GLEE_ARB_pixel_buffer_object;
  _supportsFP40=!!GLEE_NV_fragment_program2;
  memset(_PBOs, 0, sizeof(_PBOs));
  _PBOcount=0;
  if(_havePBOs) {
    glGenBuffers(sizeof(_PBOs)/sizeof(GLuint), _PBOs);
    CHECK_GL();
  }

  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &image_units);
  glGetIntegerv(GL_MAX_TEXTURE_COORDS, &tex_coords);
  
  GPUAssert (tex_coords <= image_units,
             "So sad, you have more texture coordinates that textures");
  _slopTextureUnit = (unsigned int) (image_units - 1);

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *) &_maxTextureExtent);

  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, (GLint *) &_maxOutputCount);
  GPUAssert (_maxOutputCount <= 32,
             "Maximum FBO attachments seems higher than OpenGL spec");
}

OGLContext::OGLContext():
  _passthroughVertexShader(0),
  _passthroughPixelShader(0), 
  _slopTextureUnit(0),
  _maxTextureExtent(1024),
  _maxOutputCount(1), 
  _boundPixelShader(NULL), 
  _wnd(NULL)
{
  int i;
  for (i=0; i<32; i++) {
    _outputTextures[i] = NULL;
    _outputTexturesCache[i] = NULL;
    _boundTextures[i] = NULL;
  }
}

bool OGLContext::isTextureExtentValid( unsigned int inExtent ) const {
  return inExtent <= _maxTextureExtent;
}

unsigned int OGLContext::getMaximumOutputCount() const {
  return _maxOutputCount;
}

int OGLContext::getShaderFormatRank (const char *name) const {
  if( strcmp(name, "arb") == 0 )
    return 1;
  if( _supportsFP40 &&
      strcmp(name, "fp40") == 0 )
    return 2;
  if( GLEE_VERSION_2_0 && strcmp(name, "glsl") == 0 )
    return 3;
  return -1;
}

GPUContext::TextureHandle 
OGLContext::createTexture2D( unsigned int inWidth,
                            unsigned int inHeight, 
                            GPUContext::TextureFormat inFormat, bool read_only) {
  return (GPUContext::TextureHandle) 
    new OGLTexture(this, inWidth, inHeight, inFormat);
}

OGLContext::~OGLContext() {
  if(_havePBOs) {
    glDeleteBuffers(sizeof(_PBOs)/sizeof(GLuint), _PBOs);
  }
  for(unsigned int n=0; n<_maxOutputCount && _outputTexturesCache[n]; n++)
    delete _outputTexturesCache[n];
  if (_wnd)
    delete _wnd;
}

void* OGLContext::getTextureRenderData( OGLContext::TextureHandle inTexture )
{
  OGLTexture* texture = (OGLTexture*) inTexture;
  return (void*) texture->id();
}

void OGLContext::finish()
{
   glFinish();
}

void OGLContext::unbind()
{
}

void OGLContext::bind()
{
  _wnd->makeCurrent();
}

void OGLContext::shareLists( HGLRC inContext )
{
  _wnd->shareLists( inContext );
}

IWriteQuery *OGLContext::createWriteQuery(void)
{
   return new OGLWriteQuery();
}

