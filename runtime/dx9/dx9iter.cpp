// dx9iter.cpp
#include "dx9.hpp"

using namespace brook;

static float lerp (unsigned int i, unsigned int end,float lower,float upper) {
   float frac=end>0?((float)i)/(float)end:(float)upper;
   return (1-frac)*lower+frac*upper;
}

DX9Iter::DX9Iter( DX9RunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[], 
             float ranges[])
  :Iter(type),dimensionCount(dims), elementType(type), cpuBuffer(NULL)
{
  if( dimensionCount < 0 )
    DX9Fail("Bad dimension count at iterator creation");
  if( dimensionCount > 2 )
    DX9Fail("Can't create greater than 2d iterators with DX9");

  switch( elementType )
  {
  case __BRTFLOAT:
    componentCount = 1;
    break;
  case __BRTFLOAT2:
    componentCount = 2;
    break;
  case __BRTFLOAT3:
    componentCount = 3;
    break;
  case __BRTFLOAT4:
    componentCount = 4;
    break;
  default:
    DX9Fail( "Unkown stream element type." );
    break;
  }

  totalSize = 1;
  for( int i = 0; i < dimensionCount; i++ )
  {
    this->extents[i] = extents[i];
    totalSize *= extents[i];
  }

  int rangeCount = componentCount*2;
  for( int r = 0; r < rangeCount; r++ )
    this->ranges[r] = ranges[r];

  if( dimensionCount == 1 )
  {
    float4 min = {0,0,0,0};
    float4 max = {0,0,0,0};

    int i;
    for( i = 0; i < componentCount; i++ )
    {
      ((float*)&min)[i] = ranges[i];
      ((float*)&max)[i] = ranges[i+componentCount];
    }
    // fill in remaining components just in case...
    for( ; i < 4; i++ )
    {
      ((float*)&min)[i] = (i==4) ? 1.0f : 0.0f;
      ((float*)&max)[i] = (i==4) ? 1.0f : 0.0f;
    }

    rect.vertices[0] = max;
    rect.vertices[1] = min;
    rect.vertices[2] = max;
    rect.vertices[3] = min;
  }
  else
  {
    DX9Assert( componentCount == 2, "only float2 2D iterator in DX9 right now" );
    float minX = ranges[0];
    float minY = ranges[1];
    float maxX = ranges[2];
    float maxY = ranges[3];
    rect = DX9Rect( minX, maxY, maxX, minY );
  }
}

void* DX9Iter::getData (unsigned int flags)
{
  DX9Assert( !(flags & Stream::WRITE), "Cannot write to an iterator!!" );

  if( cpuBuffer != NULL ) return cpuBuffer;

  cpuBufferSize = totalSize * componentCount * sizeof(float);

  cpuBuffer = malloc( cpuBufferSize );

  // fill in the data
  float* data = (float*)cpuBuffer;
  if( dimensionCount == 1 )
  {
    for( unsigned int i = 0; i < extents[0]; i++ )
    {
      for( int j = 0; j < componentCount; j++ )
        *data++ = lerp( i, extents[0], ranges[0], ranges[2] );
    }
  }
  else if( dimensionCount == 2 )
  {
    unsigned int i[2];
    for( i[0] = 0; i[0] < extents[0]; i[0]++ )
    {
      for( i[1] = 0; i[1] < extents[1]; i[1]++ )
      {
        for( int k = 0; k < 2; k++ )
          *data++ = lerp( i[1-k], extents[1-k], ranges[k], ranges[2+k] );
      }
    }
  }
  else
  {
    DX9Fail("can't handle > 2d iterators in DX9");
  }
  
  return cpuBuffer;
}

void DX9Iter::releaseData(unsigned int flags)
{
}
