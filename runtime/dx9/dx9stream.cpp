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
  : Stream(type), runtime(runtime)
{
  DX9Trace("DX9Stream::DX9Stream");
  // XXX: TO DO
  // for now allocate a 1D float4 texture with
  // the number of elements requested...

  int width;
  int height;
  int components = 0;

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
     components=1;
     break;
  case __BRTFLOAT2:
     components=2;
     break;
  case __BRTFLOAT3:
     components=3;
     break;
  case __BRTFLOAT4:
     components=4;
     break;
  default:
     components=0;
  }
  if( components == 0 )
  {
    DX9Trace( "DX9Stream failure, unknown element type %d\n", (int)type );
    assert(false);
  }

  texture = DX9Texture::create( runtime, width, height, components );

  inputRect = DX9Rect( 0, 1, 1, 0 );
  outputRect = DX9Rect( -1, -1, 1, 1 );

  float scaleX = 1.0f / width;
  float scaleY = 1.0f / height;
  float offsetX = 0.5f / width;
  float offsetY = 0.5f / height;
  gatherConstant.x = scaleX;
  gatherConstant.y = scaleY;
  gatherConstant.z = offsetX;
  gatherConstant.w = offsetY;
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
