// dx9iter.cpp
#include "dx9.hpp"

using namespace brook;

static float lerp (unsigned int i, unsigned int end,float lower,float upper) {
   float frac=end>0?((float)i)/(float)end:(float)upper;
   return (1-frac)*lower+frac*upper;
}

DX9Iter* DX9Iter::create( DX9RunTime* inRuntime, __BRTStreamType inElementType,
  int inDimensionCount, int* inExtents, float* inRanges )
{
  DX9PROFILE("DX9Iter::create")
  DX9Iter* result = new DX9Iter( inRuntime, inElementType );
  if( result->initialize( inDimensionCount, inExtents, inRanges ) )
    return result;
  delete result;
  return NULL;
}

DX9Iter::DX9Iter( DX9RunTime* inRuntime, __BRTStreamType inElementType )
  : Iter(inElementType), cpuBuffer(NULL)
{
}

bool DX9Iter::initialize( int inDimensionCount, int* inExtents, float* inRanges )
{
  dimensionCount = inDimensionCount;
  if( (dimensionCount <= 0) || (dimensionCount > 2) )
  {
    DX9WARN << "Invalid dimension for iterator stream "
      << inDimensionCount << ".\n"
      << "Dimension must be greater than 0 and less than 3.";
    return false;
  }

  switch( type )
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
    DX9WARN << "Unknown iterator element type.\n"
      "Element type must be one of float, float2, float3, float4.";
    return false;
    break;
  }

  totalSize = 1;
  for( int i = 0; i < dimensionCount; i++ )
  {
    int extent = inExtents[i];
    if( extent <= 0 )
    {
      DX9WARN << "Invalid iterator extent " << extent << " in dimension " << i << ".\n"
        << "The extent must be greater than 0.";
      return false;
    }

    this->extents[i] = extent;
    totalSize *= extents[i];
  }

  int rangeCount = componentCount*2;
  for( int r = 0; r < rangeCount; r++ )
    ranges[r] = inRanges[r];

  if( dimensionCount == 1 )
  {
    float4 min = float4(0,0,0,0);
    float4 max = float4(0,0,0,0);

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
    DX9Assert( dimensionCount == 2, "Internal error." );

    if( componentCount != 2 )
    {
      DX9WARN << "Invalid element type for 2D iterator.\n"
        << "2D iterators can only be of type float2.";
      return false;
    }
    float minX = ranges[0];
    float minY = ranges[1];
    float maxX = ranges[2];
    float maxY = ranges[3];
    rect = DX9Rect( minX, maxY, maxX, minY );
  }
  return true;
}

void* DX9Iter::getData (unsigned int flags)
{
  DX9PROFILE("DX9Iter::getData")
  DX9Assert( !(flags & Stream::WRITE),
    "Attempted to write to an iterator.\n"
    "Iterators are strictly read-only." );

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
        *data++ = lerp( i, extents[0], ranges[j], ranges[j+componentCount] );
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
    DX9Assert( false, "Should be unreachable" );
  }
  
  return cpuBuffer;
}

void DX9Iter::releaseData(unsigned int flags) {
  // empty
}
