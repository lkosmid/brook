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

    IDirect3DDevice9* getDevice() { return device; }

  private:
    Context( Window* inWindow );
    bool initialize();

    Window* window;
    IDirect3D9* direct3d;
    IDirect3DDevice9* device;
  };
}