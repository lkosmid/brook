// gpuiterator.cpp
#include "gpuiterator.hpp"

using namespace brook;

static float lerp (unsigned int i, unsigned int end,float lower,float upper) {
   float frac=end>0?((float)i)/(float)end:(float)upper;
   return (1-frac)*lower+frac*upper;
}

GPUIterator* GPUIterator::create( GPURuntime* inRuntime, __BRTStreamType inElementType,
  int inDimensionCount, int* inExtents, float* inRanges )
{
  GPUIterator* result = new GPUIterator( inRuntime, inElementType );
  if( result->initialize( inDimensionCount, inExtents, inRanges ) )
    return result;
  delete result;
  return NULL;
}

GPUIterator::GPUIterator( GPURuntime* inRuntime, __BRTStreamType inElementType )
  : Iter(inElementType), _cpuBuffer(NULL)
{
}

bool GPUIterator::initialize( int inDimensionCount, int* inExtents, float* inRanges )
{
  _dimensionCount = inDimensionCount;
  if( (_dimensionCount <= 0) || (_dimensionCount > 2) )
  {
    GPUWARN << "Invalid dimension for iterator stream "
      << inDimensionCount << ".\n"
      << "Dimension must be greater than 0 and less than 3.";
    return false;
  }

  switch( type )
  {
  case __BRTFLOAT:
    _componentCount = 1;
    break;
  case __BRTFLOAT2:
    _componentCount = 2;
    break;
  case __BRTFLOAT3:
    _componentCount = 3;
    break;
  case __BRTFLOAT4:
    _componentCount = 4;
    break;
  default:
    GPUWARN << "Unknown iterator element type.\n"
      "Element type must be one of float, float2, float3, float4.";
    return false;
    break;
  }

  _totalSize = 1;
  for( unsigned int i = 0; i < _dimensionCount; i++ )
  {
    int extent = inExtents[i];
    if( extent <= 0 )
    {
      GPUWARN << "Invalid iterator extent " << extent << " in dimension " << i << ".\n"
        << "The extent must be greater than 0.";
      return false;
    }

    _extents[i] = extent;
    _totalSize *= _extents[i];
  }

  int rangeCount = _componentCount*2;
  for( int r = 0; r < rangeCount; r++ )
    _ranges[r] = inRanges[r];

  if( _dimensionCount == 1 )
  {
    float4 min = float4(0,0,0,0);
    float4 max = float4(0,0,0,0);

    unsigned int i;
    for( i = 0; i < _componentCount; i++ )
    {
      ((float*)&min)[i] = _ranges[i];
      ((float*)&max)[i] = _ranges[i+_componentCount];
    }
    // fill in remaining components just in case...
    for( ; i < 4; i++ )
    {
      ((float*)&min)[i] = (i==4) ? 1.0f : 0.0f;
      ((float*)&max)[i] = (i==4) ? 1.0f : 0.0f;
    }

    _rect.vertices[0] = max;
    _rect.vertices[1] = min;
    _rect.vertices[2] = max;
    _rect.vertices[3] = min;
  }
  else
  {
    GPUAssert( _dimensionCount == 2, "Internal error." );

    if( _componentCount != 2 )
    {
      GPUWARN << "Invalid element type for 2D iterator.\n"
        << "2D iterators can only be of type float2.";
      return false;
    }
    float minX = _ranges[0];
    float minY = _ranges[1];
    float maxX = _ranges[2];
    float maxY = _ranges[3];
    _rect = GPURect( minX, maxY, maxX, minY );
  }
  return true;
}

void* GPUIterator::getData (unsigned int flags)
{
  GPUAssert( !(flags & Stream::WRITE),
    "Attempted to write to an iterator.\n"
    "Iterators are strictly read-only." );

  if( _cpuBuffer != NULL ) return _cpuBuffer;

  size_t cpuBufferSize = _totalSize * _componentCount * sizeof(float);

  _cpuBuffer = malloc( cpuBufferSize );

  // fill in the data
  float* data = (float*)_cpuBuffer;
  if( _dimensionCount == 1 )
  {
    for( unsigned int i = 0; i < _extents[0]; i++ )
    {
      for( unsigned int j = 0; j < _componentCount; j++ )
        *data++ = lerp( i, _extents[0], _ranges[j], _ranges[j+_componentCount] );
    }
  }
  else if( _dimensionCount == 2 )
  {
    unsigned int i[2];
    for( i[0] = 0; i[0] < _extents[0]; i[0]++ )
    {
      for( i[1] = 0; i[1] < _extents[1]; i[1]++ )
      {
        for( int k = 0; k < 2; k++ )
          *data++ = lerp( i[1-k], _extents[1-k], _ranges[k], _ranges[2+k] );
      }
    }
  }
  else
  {
    GPUAssert( false, "Should be unreachable" );
  }
  
  return _cpuBuffer;
}

void GPUIterator::releaseData(unsigned int flags) {
  // empty
}
