// dx9pixelshader.hpp
#pragma once

#include "dx9base.hpp"

namespace brook {

  class DX9PixelShader
  {
  public:
	  static DX9PixelShader* create( DX9RunTime* inContext, const char* inSource );
	  ~DX9PixelShader();

    IDirect3DPixelShader9* getHandle() {
      return shaderHandle;
    }

  private:
	  DX9PixelShader( DX9RunTime* inContext );
    bool initialize( const char* inSource );

    IDirect3DDevice9* device;
	  IDirect3DPixelShader9* shaderHandle;
  };

}