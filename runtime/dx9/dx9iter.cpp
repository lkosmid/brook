// dx9iter.cpp
#include "dx9.hpp"

using namespace brook;

DX9Iter::DX9Iter( DX9RunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[], 
             float ranges[])
  :Iter(type),dimensionCount(dims), elementType(type)
{
  if( dimensionCount < 0 )
    DX9Fail("Bad dimension count at iterator creation");
  if( dimensionCount > 2 )
    DX9Fail("Can't create greater than 2d iterators with DX9");

  totalSize = 1;
  for( int i = 0; i < dimensionCount; i++ )
  {
    this->extents[i] = extents[i];
    totalSize *= extents[i];
  }

  if( dimensionCount == 1 )
  {
    float min = ranges[0];
    float max = ranges[1];
    rect.left = min;
    rect.right = max;
    rect.top = 0;
    rect.bottom = 0;
  }
  else
  {
    float minX = ranges[0];
    float minY = ranges[1];
    float maxX = ranges[2];
    float maxY = ranges[3];
    rect.left = minX;
    rect.right = maxX;
    rect.top = minY;
    rect.bottom = maxY;
  }
}

void* DX9Iter::getData (unsigned int flags)
{
  DX9Fail("can't access iterator data yet");
  return NULL;
}

void DX9Iter::releaseData(unsigned int flags)
{
  DX9Fail("can't access iterator data yet");
}