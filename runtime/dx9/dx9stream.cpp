#include "dx9.hpp"

#include "dx9texture.hpp"

using namespace brook;

DX9Stream* DX9Stream::create( DX9RunTime* inRuntime,
  int inFieldCount, const __BRTStreamType* inFieldTypes,
  int inDimensionCount, const int* inExtents )
{
  DX9Stream* result = new DX9Stream( inRuntime );
  if( result->initialize( inFieldCount, inFieldTypes, inDimensionCount, inExtents ) )
    return result;
  delete result;
  return NULL;
}

DX9Stream::DX9Stream( DX9RunTime* inRuntime )
  : runtime(inRuntime), systemDataBuffer(0), systemDataBufferSize(0), systemDataChanged(false), gpuDataChanged(false)
{
}

bool DX9Stream::initialize(
  int inFieldCount, const __BRTStreamType* inFieldTypes,
  int inDimensionCount, const int* inExtents )
{
  dimensionCount = inDimensionCount;
  if( (dimensionCount <= 0) || (dimensionCount > 2) )
  {
    DX9Warn("Unable to create stream with %d dimensions.\n"
      "Dimensions must be greater than 0 and less than 3.", dimensionCount );
    return false;
  }

  totalSize = 1;
  for( unsigned int d = 0; d < dimensionCount; d++ )
  {
    int extent = inExtents[d];
    if( (extent <= 0) || (extent > kDX9MaximumTextureSize) )
    {
      DX9Warn("Unable to create stream with extent %d in dimension %d.\n"
        "Extent must be greater than 0 and less than or equal to %d.",
        extent, d, kDX9MaximumTextureSize );
      return false;
    }
    extents.push_back(extent);
    totalSize *= extents[d];
  }

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

  for( int i = 0; i < inFieldCount; i++ )
  {
    __BRTStreamType fieldType = inFieldTypes[i];
    int fieldComponentCount;

    switch (fieldType) {
    case __BRTFLOAT:
      fieldComponentCount=1;
      break;
    case __BRTFLOAT2:
      fieldComponentCount=2;
      break;
    case __BRTFLOAT3:
      fieldComponentCount=3;
      break;
    case __BRTFLOAT4:
      fieldComponentCount=4;
      break;
    default:
      DX9Warn( "Invalid element type for stream.\n",
        "Only float, float2, float3 and float4 elements are supported." );
      return false;
    }

    DX9Texture* fieldTexture = DX9Texture::create( runtime, width, height, fieldComponentCount );
    if( fieldTexture == NULL )
    {
      DX9Warn( "Texture allocation failed during sream initialization." );
      return false;
    }

    fields.push_back( Field(fieldType,fieldComponentCount,fieldTexture) );
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
  int fieldCount = (int)fields.size();
  for( int f = 0; f < fieldCount; f++ )
  {
    delete fields[f].texture;
  }
}

void DX9Stream::Read(const void *p) {
  if( systemDataBuffer != 0 && !gpuDataChanged )
  {
    memcpy( systemDataBuffer, p, systemDataBufferSize );
    markSystemDataChanged();
  }
  else
  {
    ReadImpl( p );
    systemDataChanged = false;
    gpuDataChanged = true;
  }
}

void DX9Stream::Write(void *p) {
  if( systemDataBuffer != 0 && !gpuDataChanged )
  {
    memcpy( p, systemDataBuffer, systemDataBufferSize );
  }
  else
  {
    WriteImpl( p );
  }
}

void DX9Stream::ReadImpl(const void* inData)
{
  const char* data = (const char*)inData;
  unsigned int stride = getElementSize();
  int fieldCount = (int)fields.size();
  for( int f = 0; f < fieldCount; f++ )
  {
    fields[f].texture->setData( (const float*)data, stride );
    data += fields[f].componentCount * sizeof(float);
  }
}

void DX9Stream::WriteImpl(void* outData)
{
  char* data = (char*)outData;
  unsigned int stride = getElementSize();
  int fieldCount = (int)fields.size();
  for( int f = 0; f < fieldCount; f++ )
  {
    fields[f].texture->getData( (float*)data, stride );
    data += fields[f].componentCount * sizeof(float);
  }
}

int DX9Stream::getWidth() {
  return width;
}

int DX9Stream::getHeight() {
  return height;
}

int DX9Stream::getSubstreamCount() {
  return (int)fields.size();
}

DX9Texture* DX9Stream::getIndexedTexture( int inIndex ) {
  return fields[inIndex].texture;
}

IDirect3DTexture9* DX9Stream::getIndexedTextureHandle( int inIndex ) {
  return fields[inIndex].texture->getTextureHandle();
}

IDirect3DSurface9* DX9Stream::getIndexedSurfaceHandle( int inIndex ) {
  return fields[inIndex].texture->getSurfaceHandle();
}

DX9Rect DX9Stream::getTextureSubRect( int l, int t, int r, int b ) {
  DX9Assert( fields.size() > 0, "internal failure" );
  return fields[0].texture->getTextureSubRect( l, t, r, b );
}

DX9Rect DX9Stream::getSurfaceSubRect( int l, int t, int r, int b ) {
  DX9Assert( fields.size() > 0, "internal failure" );
  return fields[0].texture->getSurfaceSubRect( l, t, r, b );
}

void* DX9Stream::getData (unsigned int flags)
{
  if( systemDataBuffer == 0 )
  {
    systemDataBufferSize = getTotalSize() * getElementSize();
    systemDataBuffer = new char[ systemDataBufferSize ];
  }

  if( flags & Stream::READ )
    validateSystemData();
  return (void*)systemDataBuffer;
}

void DX9Stream::releaseData(unsigned int flags)
{
  if( flags & Stream::WRITE )
    markSystemDataChanged();
}

void DX9Stream::validateSystemData()
{
  if( !gpuDataChanged ) return;
  this->WriteImpl( systemDataBuffer );
  gpuDataChanged = false;
}

void DX9Stream::markSystemDataChanged()
{
  gpuDataChanged = false;
  systemDataChanged = true;
}

void DX9Stream::validateGPUData()
{
  if( systemDataChanged )
  {
    this->ReadImpl( systemDataBuffer );
    systemDataChanged = false;
  }

  int fieldCount = (int)fields.size();
  for( int f = 0; f < fieldCount; f++ )
    fields[f].texture->validateCachedData();
}

void DX9Stream::markGPUDataChanged()
{
  systemDataChanged = false;
  gpuDataChanged = true;

  int fieldCount = (int)fields.size();
  for( int f = 0; f < fieldCount; f++ )
    fields[f].texture->markCachedDataChanged();
}