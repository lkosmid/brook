#include <dx9.hpp>

#include <dx9texture.hpp>

DX9Stream::DX9Stream (DX9RunTime* runtime, const char type[], int dims, int extents[])
  : __BrookStream(type, dims, extents), runtime(runtime)
{
  DX9Trace("DX9Stream::DX9Stream");
  // XXX: TO DO
  // for now allocate a 1D float4 texture with
  // the number of elements requested...

  int size = 1;
  for( int d = 0; d < dims; d++ )
    size *= extents[d];

  texture = DX9Texture::create( runtime, size, 1 );
}

void DX9Stream::streamRead(void *p) {
  DX9Trace("streamRead");
  texture->setData( (float*)p );
}

void DX9Stream::streamWrite(void *p) {
  DX9Trace("streamWrite");
  texture->getData( (float*)p );
}

DX9Stream::~DX9Stream () {
  // Does nothing
}
