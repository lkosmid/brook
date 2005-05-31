// context.cpp
#pragma once

#include "core.hpp"
#include "context.hpp"

using namespace fibble;

Context* Context::create( Window* inWindow )
{
  Context* result = new Context( inWindow );
  if( result->initialize() )
    return result;
  delete result;
  return NULL;
}

Context::Context( Window* inWindow )
  : window( inWindow )
{
}

Context::~Context()
{
}

bool Context::initialize()
{
  HWND windowHandle = window->getWindowHandle();

  _deviceContext = GetDC( windowHandle );

  int status;
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
      1,                              // version number
      PFD_DRAW_TO_WINDOW |            // support window
      PFD_SUPPORT_OPENGL |            // support OpenGL
      PFD_DOUBLEBUFFER,               // double buffered
      PFD_TYPE_RGBA,                  // RGBA type
      0,                              // 24-bit color depth
      0, 0, 0, 0, 0, 0,               // color bits ignored
      0,                              // no alpha buffer
      0,                              // shift bit ignored
      0,                              // no accumulation buffer
      0, 0, 0, 0,                     // accum bits ignored
      0,                              // set depth buffer
      0,                              // set stencil buffer
      0,                              // no auxiliary buffer
      PFD_MAIN_PLANE,                 // main layer
      0,                              // reserved
      0, 0, 0                         // layer masks ignored
  };
  int pixelformat;

  pixelformat = ChoosePixelFormat( _deviceContext, &pfd );

  GLAssert(pixelformat, "ChoosePixelFormat failed");

  status = SetPixelFormat( _deviceContext, pixelformat, &pfd );

  GLAssert(status, "SetPixelFormat failed");

  _glContext = wglCreateContext(_deviceContext);
  GLAssert(_glContext, "Unable to create window GL context");

  status = wglMakeCurrent(_deviceContext, _glContext);
  GLAssert(status, "Unable to make current the window GL context");

  return true;
}

void Context::clear()
{
  glClear( GL_COLOR_BUFFER_BIT );
}

void Context::swap()
{
  SwapBuffers( _deviceContext );
}

bool Context::beginScene()
{
  return true;
}

void Context::endScene()
{
}

void Context::bind()
{
  wglMakeCurrent(_deviceContext, _glContext);
}

VertexShader* Context::createVertexShader( const std::string& inSource )
{
  return new VertexShader( this, inSource );
}

PixelShader* Context::createPixelShader( const std::string& inSource )
{
  return new PixelShader( this, inSource );
}

void Context::setVertexShader( VertexShader* inShader )
{
   inShader->bind();
}

void Context::setPixelShader( PixelShader* inShader )
{
   inShader->bind();
}
