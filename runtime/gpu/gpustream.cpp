#include "gpu.hpp"

namespace brook
{

  GPUStream* 
  GPUStream::create( GPURuntime* inRuntime,
                     int inFieldCount, 
                     const __BRTStreamType* inFieldTypes,
                     int inDimensionCount, 
                     const int* inExtents )
  {
    GPUStream* result = new GPUStream( inRuntime );
    if( result->initialize( inFieldCount, 
                            inFieldTypes, 
                            inDimensionCount, 
                            inExtents ) )
      return result;
    delete result;
    return NULL;
  }

  
  GPUStream::GPUStream( GPURuntime* inRuntime )
    : _runtime(inRuntime), _context(NULL), _cpuData(0), _cpuDataSize(0)
  {
    _context = _runtime->getContext();
  }

  

  void
  GPUStream::getStreamInterpolant (unsigned int _textureWidth,
                                   unsigned int _textureHeight,
                                   GPUInterpolant &_interpolant) {
    
    _context->getStreamInterpolant( getIndexedFieldTexture( 0 ),
                                    _textureWidth,
                                    _textureHeight,
                                    _interpolant);
  }


  bool 
  GPUStream::initialize( int inFieldCount, 
                         const __BRTStreamType* inFieldTypes,
                         int inDimensionCount, 
                         const int* inExtents )
  {
    _dimensionCount = (unsigned int)inDimensionCount;
    if( _dimensionCount == 0 ||
        _dimensionCount > 2)
      {
        GPUWARN << "Unable to create stream with " << _dimensionCount << " dimensions.\n"
                << "Dimensions must be either 1 or 2.";
        return false;
      }
    
    _totalSize = 1;
    unsigned int d;
    for( d = 0; d < _dimensionCount; d++ )
      {
        unsigned int extent = (unsigned int)inExtents[d];
        if( !_context->isTextureExtentValid( extent ) )
          {
            GPUWARN << "Unable to create stream with extent " << extent
                    << " in dimension " << d << ".";
            return false;
          }
        _extents.push_back(extent);
        _totalSize *= _extents[d];
      }
    
    d = _dimensionCount;
    while( d-- > 0 )
      {
        _reversedExtents.push_back(_extents[d]);
      }
    
    switch( _dimensionCount )
      {
      case 1:
        _textureWidth = _extents[0]>0?_extents[0]:1;
        _textureHeight = 1;
        break;
      case 2:
        _textureWidth = _extents[1]>0?_extents[1]:1;
        _textureHeight = _extents[0]>0?_extents[0]:1;
        break;
      default:
        GPUAssert( false, "Should be unreachable" );
        return false;
      }
    
    for( int i = 0; i < inFieldCount; i++ )
      {
        __BRTStreamType fieldType = inFieldTypes[i];
        TextureFormat fieldTextureFormat;
        size_t fieldComponentCount;
        
        switch (fieldType) {
        case __BRTFLOAT:
          fieldComponentCount=1;
          fieldTextureFormat = GPUContext::kTextureFormat_Float1;
          break;
        case __BRTFLOAT2:
          fieldComponentCount=2;
          fieldTextureFormat = GPUContext::kTextureFormat_Float2;
          break;
        case __BRTFLOAT3:
          fieldComponentCount=3;
          fieldTextureFormat = GPUContext::kTextureFormat_Float3;
          break;
        case __BRTFLOAT4:
          fieldComponentCount=4;
          fieldTextureFormat = GPUContext::kTextureFormat_Float4;
          break;
        default:
          GPUWARN << "Invalid element type for stream.\n"
                  << "Only float, float2, float3 and float4 elements are supported.";
          return false;
        }
        
        TextureHandle fieldTexture = 
          _context->createTexture2D( _textureWidth, 
                                     _textureHeight, 
                                     fieldTextureFormat );
        if( fieldTexture == NULL )
          {
            GPUWARN << "Texture allocation failed during stream initialization.";
            return false;
          }
        
        _fields.push_back( Field(fieldType,
                                 fieldComponentCount,
                                 fieldTexture) );
      }
    
    _context->getStreamInterpolant( getIndexedFieldTexture( 0 ),
                                    _textureWidth,
                                    _textureHeight,
                                    _defaultInterpolant);
    
    _context->getStreamOutputRegion( getIndexedFieldTexture( 0 ),
                                     _outputRegion);

    _indexofConstant = _context->getStreamIndexofConstant( getIndexedFieldTexture( 0 ) );
    _gatherConstant = _context->getStreamGatherConstant( getIndexedFieldTexture( 0 ) );
    
    return true;
  }


  GPUStream::~GPUStream ()
  {
    if( _context )
    {
      size_t fieldCount = _fields.size();
      for( size_t f = 0; f < fieldCount; f++ )
      {
        _context->releaseTexture( _fields[f].texture );
      }
    }
    if( _cpuData )
    {
      delete[] (unsigned char*)_cpuData;
    }
  }


  void 
  GPUStream::Read( const void* inData )
  {
    const unsigned char* data = (const unsigned char*) inData;
    size_t stride = getElementSize();
    size_t fieldCount = _fields.size();
    for( size_t f = 0; f < fieldCount; f++ )
    {
      _context->setTextureData( _fields[f].texture, (const float*)data, stride, _totalSize );
      data += _fields[f].componentCount * sizeof(float);
    }
  }


  void 
  GPUStream::Write( void* outData )
  {
    unsigned char* data = (unsigned char*) outData;
    size_t stride = getElementSize();
    size_t fieldCount = _fields.size();
    for( size_t f = 0; f < fieldCount; f++ )
    {
      _context->getTextureData( _fields[f].texture, (float*)data, stride, _totalSize );
      data += _fields[f].componentCount * sizeof(float);
    }
  }


  void * 
  GPUStream::getData (unsigned int flags)
  {
    if( _cpuData == NULL )
    {
      _cpuDataSize = _totalSize * getElementSize();
      _cpuData = new unsigned char[ _cpuDataSize ];
    }

    if( flags & Stream::READ )
      Write( _cpuData );
    return _cpuData;
  }


  void GPUStream::releaseData(unsigned int flags)
  {
    if( flags & Stream::WRITE )
    {
      Read( _cpuData );
    }
  }


  GPUStream::TextureHandle 
  GPUStream::getIndexedFieldTexture( size_t inIndex )
  {
    return _fields[inIndex].texture;
  }

}
