// dx9vertexshader.hpp
#pragma once

#include "dx9base.hpp"

namespace brook {

  class DX9VertexShader
  {
  public:
	  static DX9VertexShader* create( DX9RunTime* inContext, const char* inSource );
	  ~DX9VertexShader();

    IDirect3DVertexShader9* getHandle() {
      return shaderHandle;
    }

  private:
	  DX9VertexShader( DX9RunTime* inContext );
    bool initialize( const char* inSource );

    IDirect3DDevice9* device;
	  IDirect3DVertexShader9* shaderHandle;
  };

}