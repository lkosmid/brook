// dx9pixelshader.hpp
#pragma once

#include "dx9base.hpp"

namespace brook {

  class DX9PixelShader
  {
  public:
	  static DX9PixelShader* create( DX9RunTime* inContext, const char* inSource );
	  ~DX9PixelShader();

	  LPDIRECT3DPIXELSHADER9 getHandle();

  private:
	  DX9PixelShader( DX9RunTime* inContext, const char* inSource );

	  LPDIRECT3DPIXELSHADER9 shaderHandle;
  };

}