// dx9texture.cpp
#include "dx9texture.hpp"

#include "dx9.hpp"

using namespace brook;

DX9Texture::DX9Texture( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents )
	: width(inWidth),
  height(inHeight),
  components(inComponents),
  internalComponents(inComponents),
  systemDataBuffer(NULL),
  dirtyFlags(kSystemDataDirty),
  device(NULL),
  textureHandle(NULL),
  surfaceHandle(NULL),
  shadowSurface(NULL)
{
	device = inContext->getDevice();
  device->AddRef();
}

bool DX9Texture::initialize()
{
  HRESULT result;

  D3DFORMAT dxFormat;
  switch( components )
  {
  case 1:
    dxFormat = D3DFMT_R32F;
    break;
  case 2:
    dxFormat = D3DFMT_G32R32F;
    break;
  case 3:
    dxFormat = D3DFMT_A32B32G32R32F;
    internalComponents = 4;
    break;
  case 4:
    dxFormat = D3DFMT_A32B32G32R32F;
    break;
  default:
    DX9Warn( "Invalid component count %d for texture.\n",
      "Only 1,2,3 and 4-component floating point textures are supported." );
    return false;
  }

	result = device->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, dxFormat, D3DPOOL_DEFAULT, &textureHandle, NULL );
  if( FAILED( result ) )
  {
    DX9Warn( "Unable to create floating-point render target texture of size %d x %d.", width, height );
    return false;
  }
	result = textureHandle->GetSurfaceLevel( 0, &surfaceHandle );
	DX9AssertResult( result, "GetSurfaceLevel failed" );

	result = device->CreateOffscreenPlainSurface( width, height, dxFormat, D3DPOOL_SYSTEMMEM, &shadowSurface, NULL );
  if( FAILED( result ) )
  {
    DX9Warn( "Unable to create floating-point plain surface of size %d x %d.", width, height );
    return false;
  }
	return true;
}

DX9Texture::~DX9Texture()
{
  if( systemDataBuffer != NULL )
    delete systemDataBuffer;
  if( shadowSurface != NULL )
    shadowSurface->Release();
  if( surfaceHandle != NULL )
    surfaceHandle->Release();
  if( textureHandle != NULL )
    textureHandle->Release();
  if( device != NULL )
    device->Release();
}

DX9Texture* DX9Texture::create( DX9RunTime* inContext, int inWidth, int inHeight, int inComponents  )
{
  DX9Texture* result = new DX9Texture( inContext, inWidth, inHeight, inComponents );
  if( result->initialize() )
    return result;
  delete result;
  return NULL;
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
	DX9AssertResult( result, "Failed to copy floating-point render target to plain surface." );
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
	DX9AssertResult( result, "Failed to copy floating-point plain surface to render target." );
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
	DX9AssertResult( result, "LockRect failed" );

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
	DX9AssertResult( result, "UnlockRect failed" );
}

void DX9Texture::setShadowData( const void* inData )
{
  HRESULT result;
	D3DLOCKED_RECT info;

	result = shadowSurface->LockRect( &info, NULL, 0 );
	DX9AssertResult( result, "LockRect failed" );

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
	DX9AssertResult( result, "UnlockRect failed" );
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
	DX9AssertResult( result, "Failed to copy floating-point render target to plain surface" );

	D3DLOCKED_RECT info;
	result = shadowSurface->LockRect( &info, NULL, D3DLOCK_READONLY );
	DX9AssertResult( result, "LockRect failed" );

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
	DX9AssertResult( result, "UnlockRect failed" );
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

