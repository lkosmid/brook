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

	  void setData( const float* inData, unsigned int inStride );
	  void getData( float* outData, unsigned int inStride );

    void markCachedDataChanged();
    void markShadowDataChanged();
    void validateCachedData();

    DX9Rect getTextureSubRect( int l, int t, int r, int b );
    DX9Rect getSurfaceSubRect( int l, int t, int r, int b );
    DX9Rect getInterlacedTextureSubRect( int l, int t, int r, int b, int ix, int iy );
    void getPixelAt( int x, int y, float4& outResult );
    DX9Rect getReductionTextureSubRect( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax,
      int axisInterlace, int otherInterlace, int axis );
    DX9Rect getReductionSurfaceSubRect( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax, int axis );

    LPDIRECT3DTEXTURE9 getTextureHandle() {
      return textureHandle;
    }

    LPDIRECT3DSURFACE9 getSurfaceHandle() {
      return surfaceHandle;
    }

  private:
	  DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents );
    bool initialize();

    void flushCachedToShadow();
    void flushShadowToCached();
    void getShadowData( void* outData, unsigned int inStride );
    void setShadowData( const void* inData, unsigned int inStride );

	  LPDIRECT3DDEVICE9 device;

	  int width;
	  int height;
    int components;
    int internalComponents;
	  LPDIRECT3DTEXTURE9 textureHandle;
	  LPDIRECT3DSURFACE9 surfaceHandle;
  	
	  LPDIRECT3DSURFACE9 shadowSurface;

    enum DirtyFlag {
      kShadowDataDirty = 0x01,
      kCachedDataDirty = 0x02
    };
    int dirtyFlags;
  };
}