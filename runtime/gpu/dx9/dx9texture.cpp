// dx9texture.cpp
#include "dx9texture.hpp"

using namespace brook;

DX9Texture::DX9Texture( GPUContextDX9* inContext, int inWidth, int inHeight, int inComponents )
	: width(inWidth),
  height(inHeight),
  components(inComponents),
  internalComponents(inComponents),
  dirtyFlags(0),
  device(NULL),
  textureHandle(NULL),
  surfaceHandle(NULL),
  shadowSurface(NULL)
{
    _context = inContext;
	device = _context->getDevice();
    device->AddRef();
}

static D3DFORMAT getFormatForComponentCount( int inComponentCount )
{
  switch( inComponentCount )
  {
  case 1:
    return D3DFMT_R32F;
  case 2:
    return D3DFMT_G32R32F;
  case 4:
    return D3DFMT_A32B32G32R32F;
  default:
    return D3DFMT_UNKNOWN;
  }
}

bool DX9Texture::initialize()
{
  HRESULT result;

  D3DFORMAT dxFormat;
  bool validFormat = false;
  for( int i = components; i <= 4 && !validFormat; i++ )
  {
      dxFormat = getFormatForComponentCount( i );
      if( dxFormat != D3DFMT_UNKNOWN
          && _context->isRenderTextureFormatValid( dxFormat ) )
      {
          validFormat = true;
          internalComponents = i;
      }
  }
  if( !validFormat )
  {
    DX9WARN << "Could not find supported floating-point texture format." << std::endl;
    return false;
  }

	result = device->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, dxFormat, D3DPOOL_DEFAULT, &textureHandle, NULL );
  if( FAILED( result ) )
  {
    DX9WARN << "Unable to create floating-point render target texture of size "
      << width << " by " << height << " by float" << components << ".";
    return false;
  }
	result = textureHandle->GetSurfaceLevel( 0, &surfaceHandle );
	DX9AssertResult( result, "GetSurfaceLevel failed" );

	result = device->CreateOffscreenPlainSurface( width, height, dxFormat, D3DPOOL_SYSTEMMEM, &shadowSurface, NULL );
  if( FAILED( result ) )
  {
    DX9WARN << "Unable to create floating-point plain surface of size "
      << width << " by " << height << ".";
    return false;
  }
	return true;
}

DX9Texture::~DX9Texture()
{
  DX9LOG(2) << "~DX9Texture";
  if( shadowSurface != NULL )
    shadowSurface->Release();
  if( surfaceHandle != NULL )
    surfaceHandle->Release();
  if( textureHandle != NULL )
    textureHandle->Release();
  if( device != NULL )
    device->Release();
}

DX9Texture* DX9Texture::create( GPUContextDX9* inContext, int inWidth, int inHeight, int inComponents  )
{
  DX9PROFILE("DX9Texture::create")
  DX9Texture* result = new DX9Texture( inContext, inWidth, inHeight, inComponents );
  if( result->initialize() )
    return result;
  delete result;
  return NULL;
}

void DX9Texture::setData( const float* inData, unsigned int inStride, unsigned int inCount  )
{
  DX9PROFILE("DX9Texture::setData")

	setShadowData( inData, inStride, inCount  );
  markShadowDataChanged();
}

void DX9Texture::getData( float* outData, unsigned int inStride, unsigned int inCount  )
{
  DX9PROFILE("DX9Texture::getData")

  if( dirtyFlags & kShadowDataDirty )
    flushCachedToShadow();
  getShadowData( outData, inStride, inCount  );
}

void DX9Texture::markCachedDataChanged()
{
  dirtyFlags = kShadowDataDirty;
}

void DX9Texture::markShadowDataChanged()
{
  dirtyFlags = kCachedDataDirty;
}

void DX9Texture::validateCachedData()
{
  if( !(dirtyFlags & kCachedDataDirty) ) return;
  flushShadowToCached();
}

void DX9Texture::flushCachedToShadow()
{
  DX9PROFILE("DX9Texture::flushCachedToShadow")

  HRESULT result = device->GetRenderTargetData( surfaceHandle, shadowSurface );
	DX9AssertResult( result, "Failed to copy floating-point render target to plain surface." );
  dirtyFlags &= ~kShadowDataDirty;
}

void DX9Texture::flushShadowToCached()
{
  DX9PROFILE("DX9Texture::flushShadowToCached")

  HRESULT result = device->UpdateSurface( shadowSurface, NULL, surfaceHandle, NULL );
	DX9AssertResult( result, "Failed to copy floating-point plain surface to render target." );
  dirtyFlags &= ~kCachedDataDirty;
}

void DX9Texture::getShadowData( void* outData, unsigned int inStride, unsigned int inCount  )
{
  DX9PROFILE("DX9Texture::getShadowData")

  HRESULT result;

	D3DLOCKED_RECT info;
	result = shadowSurface->LockRect( &info, NULL, D3DLOCK_READONLY );
	DX9AssertResult( result, "LockRect failed" );

	int pitch = info.Pitch;
	if( pitch % 4 != 0 )
		throw 1;
	int pitchFloats = pitch / 4;
	const float* inputLine = (const float*)info.pBits;

	char* outputPixel = (char*)outData;
  unsigned int count = 0;

	for( int y = 0; y < height; y++ )
	{
		const float* inputPixel = inputLine;
		for( int x = 0; x < width; x++ )
		{
      count++;
      if( count > inCount ) break;
      const float* input = inputPixel;
      float* output = (float*)outputPixel;
			for( int c = 0; c < components; c++ )
			{
				*output++ = *input++;
			}
      inputPixel += internalComponents;
      outputPixel += inStride;
		}
		inputLine += pitchFloats;
	}

	result = shadowSurface->UnlockRect();
	DX9AssertResult( result, "UnlockRect failed" );
}

void DX9Texture::setShadowData( const void* inData, unsigned int inStride, unsigned int inCount  )
{
  DX9PROFILE("DX9Texture::setShadowData")

  HRESULT result;
	D3DLOCKED_RECT info;

	result = shadowSurface->LockRect( &info, NULL, 0 );
	DX9AssertResult( result, "LockRect failed" );

	int pitch = info.Pitch;
	if( pitch % 4 != 0 )
		throw 1;
	int pitchFloats = pitch / 4;
	float* outputLine = (float*)info.pBits;

	const char* inputPixel = (const char*)inData;
  unsigned int count = 0;

	for( int y = 0; y < height; y++ )
	{
		float* outputPixel = outputLine;
		for( int x = 0; x < width; x++ )
		{
      count++;
      if( count > inCount ) break;
      const float* input = (const float*)inputPixel;
      float* output = outputPixel;
			for( int c = 0; c < components; c++ )
			{
				*output++ = *input++;
			}
      inputPixel += inStride;
      outputPixel += internalComponents;
		}
		outputLine += pitchFloats;
	}

	result = shadowSurface->UnlockRect();
	DX9AssertResult( result, "UnlockRect failed" );
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
