#include "dx9.hpp"

#include "dx9texture.hpp"

using namespace brook;

DX9Stream* DX9Stream::create( DX9RunTime* inRuntime, __BRTStreamType inElementType,
  int inDimensionCount, int* inExtents )
{
  DX9Stream* result = new DX9Stream( inRuntime, inElementType );
  if( result->initialize( inDimensionCount, inExtents ) )
    return result;
  delete result;
  return NULL;
}

DX9Stream::DX9Stream( DX9RunTime* inRuntime, __BRTStreamType inElementType)
  : Stream(inElementType),
  runtime(inRuntime),
  texture(NULL)
{
}

bool DX9Stream::initialize( int inDimensionCount, int* inExtents )
{
  dimensionCount = inDimensionCount;
  if( (dimensionCount <= 0) || (dimensionCount > 2) )
  {
    DX9Warn("Unable to create stream with %d dimensions.\n"
      "Dimensions must be greater than 0 and less than 3.", dimensionCount );
    return false;
  }

  totalSize = 1;
  for( int d = 0; d < dimensionCount; d++ )
  {
    int extent = inExtents[d];
    if( (extent <= 0) || (extent > kDX9MaximumTextureSize) )
    {
      DX9Warn("Unable to create stream with extent %d in dimension %d.\n"
        "Extent must be greater than 0 and less than or equal to %d.",
        extent, d, kDX9MaximumTextureSize );
      return false;
    }
    extents[d] = extent;
    totalSize *= extents[d];
  }

  int width;
  int height;

  switch( dimensionCount )
  {
  case 1:
    width = extents[0];
    height = 1;
    break;
  case 2:
    width = extents[1];
    height = extents[0];
    break;
  default:
    DX9Assert( false, "Should be unreachable" );
    return false;
  }

  int i = 0;
  switch (type) {
  case __BRTFLOAT:
     componentCount=1;
     break;
  case __BRTFLOAT2:
     componentCount=2;
     break;
  case __BRTFLOAT3:
     componentCount=3;
     break;
  case __BRTFLOAT4:
     componentCount=4;
     break;
  default:
    DX9Warn( "Invalid element type for stream.\n",
      "Only float, float2, float3 and float4 elements are supported." );
    return false;
  }

  texture = DX9Texture::create( runtime, width, height, componentCount );
  if( texture == NULL )
  {
    DX9Warn( "Texture allocation failed during sream initialization." );
    return false;
  }

  inputRect = DX9Rect( 0, 1, 1, 0 );
  outputRect = DX9Rect( -1, -1, 1, 1 );

  float scaleX = 1.0f / (width);
  float scaleY = 1.0f / (height);
  float offsetX = 1.0f / (1 << 15);//0.5f / width;
  float offsetY = 1.0f / (1 << 15);//0.5f / height;
  gatherConstant.x = scaleX;
  gatherConstant.y = scaleY;
  gatherConstant.z = offsetX;
  gatherConstant.w = offsetY;

  indexofConstant.x = (float)width;
  indexofConstant.y = (float)height;
  indexofConstant.z = 0;
  indexofConstant.w = 0;

  return true;
}

DX9Stream::~DX9Stream () {
  if( texture != NULL )
    delete texture;
}

void DX9Stream::Read(const void *p) {
  texture->setData( (const float*)p );
}

void DX9Stream::Write(void *p) {
  texture->getData( (float*)p );
}

IDirect3DTexture9* DX9Stream::getTextureHandle() {
  return texture->getTextureHandle();
}

IDirect3DSurface9* DX9Stream::getSurfaceHandle() {
  return texture->getSurfaceHandle();
}

int DX9Stream::getWidth() {
  return texture->getWidth();
}

int DX9Stream::getHeight() {
  return texture->getHeight();
}

DX9Rect DX9Stream::getTextureSubRect( int l, int t, int r, int b ) {
  return texture->getTextureSubRect( l, t, r, b );
}

DX9Rect DX9Stream::getSurfaceSubRect( int l, int t, int r, int b ) {
  return texture->getSurfaceSubRect( l, t, r, b );
}

void* DX9Stream::getData (unsigned int flags)
{
  void* result = texture->getSystemDataBuffer();
  if( flags & Stream::READ )
    texture->validateSystemData();
  return result;
}

void DX9Stream::releaseData(unsigned int flags)
{
  if( flags & Stream::WRITE )
    texture->markSystemDataChanged();
}

void DX9Stream::validateGPUData()
{
  texture->validateCachedData();
}

void DX9Stream::markGPUDataChanged()
{
  texture->markCachedDataChanged();
}