
#include "oglfunc.hpp"
#include "oglcontext.hpp"
#include "oglwindow.hpp"

using namespace brook;

void
OGLContext::init (const int   (*viAttribList)[4][64],
                  const float (*vfAttribList)[4][16],
                  const int   (*vpiAttribList)[4][16]) {
  int i;
  
  _wnd = new OGLWindow();

  _wnd->initPbuffer(viAttribList, vfAttribList, vpiAttribList);

  _passthroughVertexShader = NULL;
  _passthroughPixelShader = NULL;

  glEnable(GL_FRAGMENT_PROGRAM_ARB);

  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i);
  _slopTextureUnit = (unsigned int) (i-1);

  // Check to see if we are running on hardware with
  // multiple outputs
  _maxOutputCount = 1;
  const char *ext = (const char *) glGetString(GL_EXTENSIONS);
  if(strstr(ext, "GL_ATI_draw_buffers")) {
    glGetIntegerv(GL_MAX_DRAW_BUFFERS_ATI, &i);
    assert (i>0);
    if (i >= 4)
      _maxOutputCount = 4;
  }
}

OGLContext::OGLContext():
  _passthroughVertexShader(0),
  _passthroughPixelShader(0), 
  _slopTextureUnit(0),
  _maxOutputCount(1), 
  _boundPixelShader(NULL), 
  _wnd(NULL)
{
  int i;
  for (i=0; i<4; i++) 
    _outputTextures[i] = NULL;
  for (i=0; i<32; i++) 
    _boundTextures[i] = NULL;
}

unsigned int OGLContext::getMaximumOutputCount() {
  return _maxOutputCount;
}

OGLContext::~OGLContext() {
  if (_wnd)
    delete _wnd;
}

