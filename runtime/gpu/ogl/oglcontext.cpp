

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"
#include "oglwindow.hpp"

using namespace brook;

void
OGLContext::init (const int   (*viAttribList)[4][64],
                  const float (*vfAttribList)[4][16],
                  const int   (*vpiAttribList)[4][16]) {
  int i;
  
  wnd = new OGLWindow();

  wnd->initPbuffer(viAttribList, vfAttribList, vpiAttribList);

  _passthroughVertexShader = NULL;
  _passthroughPixelShader = NULL;

  glEnable(GL_FRAGMENT_PROGRAM_ARB);

  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i);
  _slopTextureUnit = (unsigned int) (i-1);
}


OGLContext::OGLContext():
  wnd(NULL), _passthroughVertexShader(0),
  _passthroughPixelShader(0), _outputTexture(NULL),
  _slopTextureUnit(0), currentPbufferComponents(-1)
{}


OGLContext::~OGLContext() {
  if (wnd)
    delete wnd;
}

