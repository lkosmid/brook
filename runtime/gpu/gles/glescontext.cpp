#include <string.h>
#include "glesfunc.hpp"
#include "glescontext.hpp"
#include "gleswindow.hpp"
#include "glestexture.hpp"
#include "gleswritequery.hpp"
#include "glescheckgl.hpp"

using namespace brook;

GLESContext *
GLESContext::create(const char* device) {
  GLESContext *ctx = new GLESContext();

  if (!ctx)
    return NULL;

  ctx->init(device);

  return ctx;
}

void
GLESContext::init (const char* device) {
  int image_units;
  int tex_coords;
  
  _wnd = new GLESWindow(device);

  _wnd->initFBO();

  _passthroughVertexShader = NULL;
  _passthroughPixelShader = NULL;

  const char *vendor = (const char *) glGetString(GL_VENDOR);
  _havePBOs= 0; //TODO Check for OpenGL ES 3.0
  memset(_PBOs, 0, sizeof(_PBOs));
  _PBOcount=0;
  if(_havePBOs) {
    glGenBuffers(sizeof(_PBOs)/sizeof(GLuint), _PBOs);
    CHECK_GL();
  }

  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &image_units);
  CHECK_GL();
  tex_coords=image_units;
  
  GPUAssert (tex_coords <= image_units,
             "So sad, you have more texture coordinates that textures");
  _slopTextureUnit = (unsigned int) (image_units - 1);

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *) &_maxTextureExtent);
  CHECK_GL();

#ifdef GLES3
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, (GLint *) &_maxOutputCount);
#endif
  _maxOutputCount=1;
  GPUAssert (_maxOutputCount <= 32,
             "Maximum FBO attachments seems higher than OpenGL spec");
}

GLESContext::GLESContext():
  _passthroughVertexShader(0),
  _passthroughPixelShader(0), 
  _slopTextureUnit(0),
  _maxTextureExtent(1024),
  _maxOutputCount(1), 
  _boundPixelShader(NULL), 
  _wnd(NULL)
{
  unsigned int i;
  for (i=0; i<MAXBOUNDTEXTURES; i++) {
    _outputTextures[i] = NULL;
    _outputTexturesCache[i] = NULL;
    _boundTextures[i] = NULL;
    _PBOs[i] = NULL;
  }
}

bool GLESContext::isTextureExtentValid( unsigned int inExtent ) const {
  return inExtent <= _maxTextureExtent;
}

unsigned int GLESContext::getMaximumOutputCount() const {
  return _maxOutputCount;
}

int GLESContext::getShaderFormatRank (const char *name) const {
  if( strcmp(name, "gles") == 0 )
    return 3;
  return -1;
}

GPUContext::TextureHandle 
GLESContext::createTexture2D( unsigned int inWidth,
                            unsigned int inHeight, 
                            GPUContext::TextureFormat inFormat, bool read_only) {
  return (GPUContext::TextureHandle) 
    new GLESTexture(this, inWidth, inHeight, inFormat);
}

GLESContext::~GLESContext() {
  if(_havePBOs) {
    glDeleteBuffers(sizeof(_PBOs)/sizeof(GLuint), _PBOs);
  }
  for(unsigned int n=0; n<_maxOutputCount && _outputTexturesCache[n]; n++)
    delete _outputTexturesCache[n];
  if (_wnd)
    delete _wnd;
}

void* GLESContext::getTextureRenderData( GLESContext::TextureHandle inTexture )
{
  GLESTexture* texture = (GLESTexture*) inTexture;
  return (void*) texture->id();
}

void GLESContext::finish()
{
   glFinish();
}

void GLESContext::unbind()
{
}

void GLESContext::bind()
{
  _wnd->makeCurrent();
}

void GLESContext::shareLists( HGLRC inContext )
{
  _wnd->shareLists( inContext );
}

IWriteQuery *GLESContext::createWriteQuery(void)
{
   return new GLESWriteQuery();
}

/* get context dependent texture width */
unsigned int 
GLESContext::get_texture_width(TextureHandle textureId) const 
{
  return ((GLESTexture*) textureId )->width();
}

/* get context dependent texture height*/
unsigned int 
GLESContext::get_texture_height(TextureHandle textureId) const 
{
  return ((GLESTexture*) textureId )->height();
}

