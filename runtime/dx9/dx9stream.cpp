#include <dx9.hpp>

#include <dx9texture.hpp>

static const char* kKnownTypeNames[] =
{
  "float",
  "float2",
  "float3",
  "float4",
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

DX9Stream::DX9Stream (DX9RunTime* runtime, const char type[], int dims, int extents[])
  : __BrookStream(type, dims, extents), runtime(runtime)
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
    width = extents[0];
    height = extents[1];
    break;
  default:
    width = 1;
    height = 1;
    for( int d = 0; d < dims; d++ )
      width *= extents[d];
    break;
  }

  int i = 0;
  while( kKnownTypeNames[i] != NULL )
  {
    const char* name = kKnownTypeNames[i];
    if( strncmp(type, name, strlen(name)) == 0 )
    {
      components = kKnownTypeComponentCounts[i];
      break;
    }
    i++;
  }
  if( components == 0 )
  {
    DX9Trace( "DX9Stream failure, unknown element type %s\n", type );
    assert(false);
  }

  texture = DX9Texture::create( runtime, width, height, components );

  inputRect = DX9Rect( 0, 1, 0, 1 );
  outputRect = DX9Rect( -1, 1, 1, -1 );
}

void DX9Stream::streamRead(void *p) {
  DX9Trace("streamRead");
  texture->setData( (float*)p );
}

void DX9Stream::streamWrite(void *p) {
  DX9Trace("streamWrite");
  texture->getData( (float*)p );
  DX9Trace("streamWrite - done");
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
