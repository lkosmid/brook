// dx9texture.hpp
#pragma once

#include "dx9base.hpp"

#define DX9TEXTURE_SHADOWTEXTURE

class DX9RunTime;

class DX9Texture
{
public:
	static DX9Texture* create( DX9RunTime* inContext, int inWidth, int inHeight );
	~DX9Texture();

	void setData( const float* inData );
	void getData( float* outData );

	LPDIRECT3DTEXTURE9 getTextureHandle();
	LPDIRECT3DSURFACE9 getSurfaceHandle();

private:
	DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight );

	LPDIRECT3DDEVICE9 device;

	int width;
	int height;
	LPDIRECT3DTEXTURE9 textureHandle;
	LPDIRECT3DSURFACE9 surfaceHandle;
	
#ifdef DX9TEXTURE_SHADOWTEXTURE
  IDirect3DTexture9* shadowTexture;
#endif
	LPDIRECT3DSURFACE9 shadowSurface;
};