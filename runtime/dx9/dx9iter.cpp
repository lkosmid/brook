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
    DX9Assert( componentCount == 1, "no more than float 1D iterator in DX9 right now" );
    
    rect.left = ranges[0];
    rect.right = ranges[1];
    rect.top = 0;
    rect.bottom = 0;
  }
  else
  {
    DX9Assert( componentCount == 2, "only float2 2D iterator in DX9 right now" );
    float minX = ranges[0];
    float minY = ranges[1];
    float maxX = ranges[2];
    float maxY = ranges[3];
    rect.left = minX;
    rect.right = maxX;
    rect.bottom = minY;
    rect.top = maxY;
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
