// dx9texture.hpp
#pragma once

#include "dx9base.hpp"

namespace brook {

  class DX9Texture
  {
  public:
	  static DX9Texture* create( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents );
	  ~DX9Texture();

    int getWidth() { return width; }
    int getHeight() { return height; }

	  void setData( const float* inData );
	  void getData( float* outData );

    DX9Rect getTextureSubRect( int l, int t, int r, int b );
    DX9Rect getSurfaceSubRect( int l, int t, int r, int b );
    void getTopLeftPixel( float4& outResult );

	  LPDIRECT3DTEXTURE9 getTextureHandle();
	  LPDIRECT3DSURFACE9 getSurfaceHandle();

  private:
	  DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents );

	  LPDIRECT3DDEVICE9 device;

	  int width;
	  int height;
    int components;
    int internalComponents;
	  LPDIRECT3DTEXTURE9 textureHandle;
	  LPDIRECT3DSURFACE9 surfaceHandle;
  	
	  LPDIRECT3DSURFACE9 shadowSurface;
  };
}