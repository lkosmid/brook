#include "dx9.hpp"

#include "dx9texture.hpp"

using namespace brook;

static const char* kKnownTypeNames[] =
{
  "float ",
  "float2 ",
  "float3 ",
  "float4 ",
  NULL
};

static int kKnownTypeComponentCounts[] =
{
  1,
  2,
  3,
  4,
  0
};

DX9Stream::DX9Stream (DX9RunTime* runtime, __BRTStreamType type, int dims, int extents[])
  : Stream(type), runtime(runtime), elementType(type), dimensionCount(dims)
{
  DX9Trace("DX9Stream::DX9Stream");
  // XXX: TO DO
  // for now allocate a 1D float4 texture with
  // the number of elements requested...

  totalSize = 1;
  for( int d = 0; d < dimensionCount; d++ )
  {
    this->extents[d] = extents[d];
    totalSize *= extents[d];
  }

  int width;
  int height;

  switch( dims )
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
    width = 1;
    height = 1;
    for( int d = 0; d < dims; d++ )
      width *= extents[d];
    break;
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
     componentCount=0;
  }
  if( componentCount == 0 )
  {
    DX9Trace( "DX9Stream failure, unknown element type %d\n", (int)type );
    assert(false);
  }

  texture = DX9Texture::create( runtime, width, height, componentCount );

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
}

void DX9Stream::Read(const void *p) {
  DX9Trace("Read");
  texture->setData( (const float*)p );
}

void DX9Stream::Write(void *p) {
  DX9Trace("Write");
  texture->getData( (float*)p );
}

DX9Stream::~DX9Stream () {
  // Does nothing
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