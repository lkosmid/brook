// context.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class Context
  {
  public:
    static Context* create( Window* inWindow );
    virtual ~Context();

    VertexShader* createVertexShader( const std::string& inSource );
    PixelShader* createPixelShader( const std::string& inSource );

    void setVertexShader( VertexShader* inShader );
    void setPixelShader( PixelShader* inShader );

    void clear();
    void swap();

    bool beginScene();
    void endScene();

    void bind();

    HGLRC getGLContext() { return _glContext; }
    HDC getDeviceContext() { return _deviceContext; }

  private:
    Context( Window* inWindow );
    bool initialize();

    Window* window;

    HDC _deviceContext;
    HGLRC _glContext;
  };
}