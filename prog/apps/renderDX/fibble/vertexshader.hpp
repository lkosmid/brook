// dx9vertexshader.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class VertexShader
  {
  public:
	  ~VertexShader();

  private:
    friend class Context;
	  VertexShader( Context* inContext, const std::string& inSource );
    IDirect3DVertexShader9* getShaderHandle() { return shaderHandle; }

    IDirect3DDevice9* device;
	  IDirect3DVertexShader9* shaderHandle;
  };

}