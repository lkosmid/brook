// dx9pixelshader.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class PixelShader
  {
  public:
	  ~PixelShader();

  private:
    friend class Context;
	  PixelShader( Context* inContext, const std::string& inSource );
    IDirect3DPixelShader9* getShaderHandle() { return shaderHandle; }

    IDirect3DDevice9* device;
	  IDirect3DPixelShader9* shaderHandle;
  };

}