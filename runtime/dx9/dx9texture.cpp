// dx9texture.cpp
#include "dx9texture.hpp"

#include "dx9.hpp"

using namespace brook;

DX9Texture::DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents )
	: width(inWidth), height(inHeight), components(inComponents), internalComponents(inComponents)
{
	device = inContext->getDevice();
	HRESULT result;

  D3DFORMAT dxFormat;
  switch( inComponents )
  {
  case 1:
    dxFormat = D3DFMT_R32F;
    break;
  case 2:
    dxFormat = D3DFMT_G32R32F;
    break;
  case 3:
    // TIM: special case - no RGB float textures...
    dxFormat = D3DFMT_A32B32G32R32F;
    internalComponents = 4;
    break;
  case 4:
    dxFormat = D3DFMT_A32B32G32R32F;
    break;
  default:
    DX9Fail("Invalid component count in DX9Texture - %d", inComponents);
  }

	result = device->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, dxFormat, D3DPOOL_DEFAULT, &textureHandle, NULL );
	DX9CheckResult( result );
	result = textureHandle->GetSurfaceLevel( 0, &surfaceHandle );
	DX9CheckResult( result );

	result = device->CreateOffscreenPlainSurface( width, height, dxFormat, D3DPOOL_SYSTEMMEM, &shadowSurface, NULL );
	DX9CheckResult( result );
}

DX9Texture::~DX9Texture()
{
	// TIM: TODO: cleanup
}

DX9Texture* DX9Texture::create( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents  )
{
	return new DX9Texture( inContext, inWidth, inHeight, inComponents );
}

void DX9Texture::setData( const float* inData )
{
	HRESULT result;
	D3DLOCKED_RECT info;

	result = shadowSurface->LockRect( &info, NULL, 0 );
	DX9CheckResult( result );

	int pitch = info.Pitch;
	if( pitch % 4 != 0 )
		throw 1;
	int pitchFloats = pitch / 4;
	float* outputLine = (float*)info.pBits;

	const float* input = inData;

	for( int y = 0; y < height; y++ )
	{
		float* outputPixel = outputLine;
		for( int x = 0; x < width; x++ )
		{
      float* output = outputPixel;
			for( int c = 0; c < components; c++ )
			{
				*output++ = *input++;
			}
      outputPixel += internalComponents;
		}
		outputLine += pitchFloats;
	}

	result = shadowSurface->UnlockRect();
	DX9CheckResult( result );

	result = device->UpdateSurface( shadowSurface, NULL, surfaceHandle, NULL );
	DX9CheckResult( result );
}

void DX9Texture::getData( float* outData )
{
	HRESULT result;

	result = device->GetRenderTargetData( surfaceHandle, shadowSurface );
	DX9CheckResult( result );

	D3DLOCKED_RECT info;
	result = shadowSurface->LockRect( &info, NULL, D3DLOCK_READONLY );
	DX9CheckResult( result );

	int pitch = info.Pitch;
	if( pitch % 4 != 0 )
		throw 1;
	int pitchFloats = pitch / 4;
	const float* inputLine = (const float*)info.pBits;

	float* output = outData;

	for( int y = 0; y < height; y++ )
	{
		const float* inputPixel = inputLine;
		for( int x = 0; x < width; x++ )
		{
      const float* input = inputPixel;
			for( int c = 0; c < components; c++ )
			{
				*output++ = *input++;
			}
      inputPixel += internalComponents;
		}
		inputLine += pitchFloats;
	}

	result = shadowSurface->UnlockRect();
	DX9CheckResult( result );
}

LPDIRECT3DTEXTURE9 DX9Texture::getTextureHandle()
{
	return textureHandle;
}

LPDIRECT3DSURFACE9 DX9Texture::getSurfaceHandle()
{
	return surfaceHandle;
}

