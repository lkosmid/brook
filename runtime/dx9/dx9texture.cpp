// dx9texture.cpp
#include "dx9texture.hpp"

#include "dx9.hpp"

using namespace brook;

DX9Texture::DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents )
	: width(inWidth), height(inHeight), components(inComponents), internalComponents(inComponents), systemDataBuffer(NULL), dirtyFlags(kSystemDataDirty)
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

  D3DSURFACE_DESC descriptor;
  result = surfaceHandle->GetDesc( &descriptor );
  DX9CheckResult( result );
  D3DFORMAT internalFormat = descriptor.Format;
  DX9Trace("components=%d internalComponents=%d dxFormat=%x, internalFormat=%x, width=%d, height=%d",
    components, internalComponents, dxFormat, internalFormat, descriptor.Width, descriptor.Height);

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
	setShadowData( inData );
  markShadowDataChanged();
}

void DX9Texture::getData( float* outData )
{
  if( !(dirtyFlags & kShadowDataDirty) )
    getShadowData( outData );
  else if( !(dirtyFlags & kSystemDataDirty) )
    memcpy( outData, systemDataBuffer, systemDataBufferSize );
  else
  {
    flushCachedToShadow();
    getShadowData( outData );
  }
}

void DX9Texture::markCachedDataChanged()
{
  dirtyFlags = kShadowDataDirty | kSystemDataDirty;
}

void DX9Texture::markShadowDataChanged()
{
  dirtyFlags = kCachedDataDirty | kSystemDataDirty;
}

void DX9Texture::markSystemDataChanged()
{
  dirtyFlags = kCachedDataDirty | kShadowDataDirty;
}

void DX9Texture::validateCachedData()
{
  if( !(dirtyFlags & kCachedDataDirty) ) return;
  if( dirtyFlags & kShadowDataDirty )
    flushSystemToShadow();
  flushShadowToCached();
}

void DX9Texture::validateSystemData()
{
  if( !(dirtyFlags & kSystemDataDirty) ) return;
  if( dirtyFlags & kShadowDataDirty )
    flushCachedToShadow();
  flushShadowToSystem();
}

void DX9Texture::flushCachedToShadow()
{
  HRESULT result = device->GetRenderTargetData( surfaceHandle, shadowSurface );
	DX9CheckResult( result );
  dirtyFlags &= ~kCachedDataDirty;
}

void DX9Texture::flushShadowToSystem()
{
  getShadowData( getSystemDataBuffer() );
  dirtyFlags &= ~kSystemDataDirty;
}

void DX9Texture::flushSystemToShadow()
{
  setShadowData( getSystemDataBuffer() );
  dirtyFlags &= ~kShadowDataDirty;
}

void DX9Texture::flushShadowToCached()
{
  HRESULT result = device->UpdateSurface( shadowSurface, NULL, surfaceHandle, NULL );
	DX9CheckResult( result );
  dirtyFlags &= ~kCachedDataDirty;
}

void* DX9Texture::getSystemDataBuffer()
{
  if( systemDataBuffer != NULL ) return systemDataBuffer;
  systemDataBufferSize = width * height * components * sizeof(float);
  systemDataBuffer = malloc( systemDataBufferSize );
  return systemDataBuffer;
}

void DX9Texture::getShadowData( void* outData )
{
  HRESULT result;

	D3DLOCKED_RECT info;
	result = shadowSurface->LockRect( &info, NULL, D3DLOCK_READONLY );
	DX9CheckResult( result );

	int pitch = info.Pitch;
	if( pitch % 4 != 0 )
		throw 1;
	int pitchFloats = pitch / 4;
	const float* inputLine = (const float*)info.pBits;

	float* output = (float*)outData;

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

void DX9Texture::setShadowData( const void* inData )
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

	const float* input = (const float*)inData;

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
}

LPDIRECT3DTEXTURE9 DX9Texture::getTextureHandle()
{
	return textureHandle;
}

LPDIRECT3DSURFACE9 DX9Texture::getSurfaceHandle()
{
	return surfaceHandle;
}

DX9Rect DX9Texture::getTextureSubRect( int l, int t, int r, int b ) {
  DX9Rect result;
  result.left = (float)(l) / (float)(width);
  result.top = (float)(b) / (float)(height);
  result.right = (float)(r) / (float)(width);
  result.bottom = (float)(t) / (float)(height);
  return result;
}

DX9Rect DX9Texture::getSurfaceSubRect( int l, int t, int r, int b ) {
  DX9Rect result;
  result.left = -1.0f + (float)(2*l) / (float)(width);
  result.top = 1.0f - (float)(2*b) / (float)(height);
  result.right = -1.0f + (float)(2*r) / (float)(width);
  result.bottom = 1.0f - (float)(2*t) / (float)(height);
  return result;
}

DX9Rect DX9Texture::getInterlacedTextureSubRect( int l, int t, int r, int b, int ix, int iy )
{
  DX9Rect result;
  result.left = (float)(l) / (float)(width);
  result.top = (float)(b) / (float)(height);
  result.right = (float)(r) / (float)(width);
  result.bottom = (float)(t) / (float)(height);
  return result;
}

void DX9Texture::getPixelAt( int x, int y, float4& outResult ) {
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
	const float* inputLine = ((const float*)info.pBits) + y*pitchFloats;

	float* output = (float*)&outResult;

	const float* inputPixel = inputLine + x*internalComponents;
	const float* input = inputPixel;

  for( int c = 0; c < components; c++ )
	{
	  *output++ = *input++;
	}

	result = shadowSurface->UnlockRect();
	DX9CheckResult( result );
}

DX9Rect DX9Texture::getReductionTextureSubRect( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax,
  int axisInterlace, int otherInterlace, int axis )
{
  int rect[4];
  int interlace[2];
  rect[0 + (axis)] = axisMin;
  rect[2 + (axis)] = axisMax;
  rect[0 + (1-axis)] = otherMin;
  rect[2 + (1-axis)] = otherMax;
  rect[0] += xOffset;
  rect[2] += xOffset;
  rect[1] += yOffset;
  rect[3] += yOffset;
  interlace[axis] = axisInterlace;
  interlace[1-axis] = otherInterlace;
  return getInterlacedTextureSubRect( rect[0], rect[1], rect[2], rect[3], interlace[0], interlace[1] );
}

DX9Rect DX9Texture::getReductionSurfaceSubRect( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax, int axis )
{
  int rect[4];
  rect[0 + (axis)] = axisMin;
  rect[2 + (axis)] = axisMax;
  rect[0 + (1-axis)] = otherMin;
  rect[2 + (1-axis)] = otherMax;
  rect[0] += xOffset;
  rect[2] += xOffset;
  rect[1] += yOffset;
  rect[3] += yOffset;
  return getSurfaceSubRect( rect[0], rect[1], rect[2], rect[3] );
}

