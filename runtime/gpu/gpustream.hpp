// gpustream.hpp
#ifndef GPU_STREAM_HPP
#define GPU_STREAM_HPP

#include "gpucontext.hpp"

namespace brook {

  class GPUStream : public Stream {
  public:
    typedef GPUContext::TextureFormat TextureFormat;
    typedef GPUContext::TextureHandle TextureHandle;

    static GPUStream* create( GPURuntime* inRuntime,
    int inFieldCount, const __BRTStreamType* inFieldTypes,
    int inDimensionCount, const int* inExtents );
    virtual void Read( const void* inData );
    virtual void Write( void* outData );

    virtual void* getData (unsigned int flags);
    virtual void releaseData(unsigned int flags);
    virtual const unsigned int* getExtents() const { return &_extents[0]; }
    virtual unsigned int getDimension() const { return _dimensionCount; }
    virtual unsigned int getTotalSize() const { return _totalSize; }
    virtual int getFieldCount() const { return (int)_fields.size(); }
    virtual __BRTStreamType getIndexedFieldType(int i) const {
      return _fields[i].elementType;
    }

    const GPURect& getInputInterpolant() {
      return _outputRect;
    }

    const GPURect& getOutputRect() {
      return _outputRect;
    }

    TextureHandle getIndexedFieldTexture( size_t inIndex );

    float4 getIndexofConstant() { return _indexofConstant; }
    float4 getShapeConstant() { return _shapeConstant; }

  private:
    GPUStream( GPURuntime* inRuntime );
    bool initialize(
      int inFieldCount, const __BRTStreamType* inFieldTypes,
      int inDimensionCount, const int* inExtents );
    virtual ~GPUStream ();

    class Field
    {
    public:
      Field( __BRTStreamType inElementType, int inComponentCount, TextureHandle inTexture )
        : elementType(inElementType), componentCount(inComponentCount), texture(inTexture)
      {}

      __BRTStreamType elementType;
      int componentCount;
      TextureHandle texture;
    };

    GPURuntime* _runtime;
    GPUContext* _context;

    unsigned int _dimensionCount;
    unsigned int _totalSize;
    std::vector<unsigned int> _extents;
    std::vector<unsigned int> _reversedExtents;
    std::vector<Field> _fields;

    unsigned int _textureWidth, _textureHeight;

    GPURect _inputRect;
    GPURect _outputRect;
    float4 _indexofConstant;
    float4 _shapeConstant;
  };
}

#endif
