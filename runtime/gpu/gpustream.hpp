// gpustream.hpp
#ifndef GPU_STREAM_HPP
#define GPU_STREAM_HPP

#include "gpucontext.hpp"

namespace brook {

    class GPUStreamData
    {
    public:
        typedef GPUContext::TextureFormat TextureFormat;
        typedef GPUContext::TextureHandle TextureHandle;

        GPUStreamData( GPURuntime* inRuntime );

        bool initialize(
            int inFieldCount, const __BRTStreamType* inFieldTypes,
            int inDimensionCount, const int* inExtents );

        void acquireReference();
        void releaseReference();

        void setData( const void* inData );
        void getData( void* outData );
        void* map(unsigned int flags);
        void unmap(unsigned int flags);

        const unsigned int* getExtents() const { return &_extents[0]; }
        unsigned int getRank() const { return _rank; }
        unsigned int getTotalSize() const { return _totalSize; }
        int getFieldCount() const { return (int)_fields.size(); }
        __BRTStreamType getIndexedFieldType(int i) const {
            return _fields[i].elementType;
        }
        unsigned int getElementSize() const {
            return _elementSize;
        }

        TextureHandle getIndexedFieldTexture( size_t inIndex ) {
            return _fields[inIndex].texture;
        }

        float4 getIndexofConstant() const { return _indexofConstant; }
        float4 getGatherConstant() const { return _gatherConstant; }

        unsigned int getTextureWidth() const {return _textureWidth; }
        unsigned int getTextureHeight() const {return _textureHeight; }

        void getOutputRegion(
            const unsigned int* inDomainMin,
            const unsigned int* inDomainMax,
            GPURegion &outRegion );


        void getStreamInterpolant(
            const unsigned int* inDomainMin,
            const unsigned int* inDomainMax,
            unsigned int inOutputWidth,
            unsigned int inOutputHeight,
            GPUInterpolant &outInterpolant );

    private:
        ~GPUStreamData();

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

        unsigned int _rank;
        unsigned int _totalSize;
        std::vector<unsigned int> _extents;
        std::vector<unsigned int> _reversedExtents;
        std::vector<Field> _fields;
        unsigned int _elementSize;

        float4 _indexofConstant, _gatherConstant;
        unsigned int _textureWidth, _textureHeight;
        GPUInterpolant _defaultInterpolant;
        GPURegion _outputRegion;

        size_t _cpuDataSize;
        void* _cpuData;

        unsigned int _referenceCount;
    };

    class GPUStream :
        public Stream
    {
    public:
        typedef GPUContext::TextureFormat TextureFormat;
        typedef GPUContext::TextureHandle TextureHandle;

        static GPUStream* GPUStream::create( GPURuntime* inRuntime,
                        int inFieldCount, 
                        const __BRTStreamType* inFieldTypes,
                        int inDimensionCount, 
                        const int* inExtents );

        GPUStream( GPUStreamData* inData );
        GPUStream( GPUStreamData* inData,
            const unsigned int* inDomainMin,
            const unsigned int* inDomainMax );

        virtual void Read( const void* inData ) {
            _data->setData( inData );
        }

        virtual void Write( void* outData ) {
            _data->getData( outData );
        }

        virtual Stream* Domain(int min, int max);
        virtual Stream* Domain(const int2& min, const int2& max);
        virtual Stream* Domain(const int3& min, const int3& max);
        virtual Stream* Domain(const int4& min, const int4& max);

        virtual void* getData (unsigned int flags);
        virtual void releaseData(unsigned int flags);

        virtual const unsigned int* getExtents() const {
            return _data->getExtents();
        }
        
        virtual unsigned int getDimension() const {
            return _data->getRank();
        }
        
        virtual unsigned int getTotalSize() const {
            return _data->getTotalSize();
        }
        
        virtual int getFieldCount() const {
            return _data->getFieldCount();
        }

        virtual __BRTStreamType getIndexedFieldType(int i) const {
            return _data->getIndexedFieldType(i);
        }

        TextureHandle getIndexedFieldTexture( size_t inIndex ) {
            return _data->getIndexedFieldTexture(inIndex);
        }

        float4 getIndexofConstant() const {
            return _data->getIndexofConstant();
        }

        float4 getGatherConstant() const {
            return _data->getGatherConstant();
        }

        unsigned int getTextureWidth() const {
            return _data->getTextureWidth();
        }

        unsigned int getTextureHeight() const {
            return _data->getTextureHeight();
        }

        void getOutputRegion( GPURegion& outRegion );

        void getStreamInterpolant( unsigned int _textureWidth,
                                   unsigned int _textureHeight,
                                   GPUInterpolant &_interpolant );


  private:
        virtual ~GPUStream ();

        enum {
            kMaximumRank = 4
        };

        GPUStreamData* _data;
        unsigned int _domainMin[kMaximumRank];
        unsigned int _domainMax[kMaximumRank];
    };

    /* TIM: must be split...
  class GPUStream : public Stream {
  public:
    typedef GPUContext::TextureFormat TextureFormat;
    typedef GPUContext::TextureHandle TextureHandle;

    static GPUStream* create( GPURuntime* inRuntime,
                              int inFieldCount, 
                              const __BRTStreamType* inFieldTypes,
                              int inDimensionCount, 
                              const int* inExtents );

    virtual void Read( const void* inData );
    virtual void Write( void* outData );
    virtual Stream* Domain(int min, int max);
    virtual Stream* Domain(const int2& min, const int2& max);
    virtual Stream* Domain(const int3& min, const int3& max);
    virtual Stream* Domain(const int4& min, const int4& max);

    virtual void* getData (unsigned int flags);
    virtual void releaseData(unsigned int flags);

    virtual const unsigned int* getExtents() const { return &_extents[0]; }
    virtual unsigned int getDimension() const { return _rank; }
    virtual unsigned int getTotalSize() const { return _totalSize; }
    virtual int getFieldCount() const { return (int)_fields.size(); }
    virtual __BRTStreamType getIndexedFieldType(int i) const {
      return _fields[i].elementType;
    }

    void getDefaultInterpolant(GPUInterpolant &interpolant) const {
      interpolant = _defaultInterpolant;
    }

    void getOutputRegion(GPURegion &region) const {
      region = _outputRegion;
    }

    void getStreamInterpolant (unsigned int _textureWidth,
                               unsigned int _textureHeight,
                               GPUInterpolant &_interpolant);

    TextureHandle getIndexedFieldTexture( size_t inIndex );

    float4 getIndexofConstant() const { return _indexofConstant; }
    float4 getGatherConstant() const { return _gatherConstant; }

    unsigned int getWidth() const {return _textureWidth; }
    unsigned int getHeight() const {return _textureHeight; }


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

    unsigned int _rank;
    unsigned int _totalSize;
    std::vector<unsigned int> _extents;
    std::vector<unsigned int> _reversedExtents;
    std::vector<Field> _fields;

    unsigned int _textureWidth, _textureHeight;

    GPUInterpolant _defaultInterpolant;
    GPURegion      _outputRegion;
    float4 _indexofConstant;
    float4 _gatherConstant;

    size_t _cpuDataSize;
    void* _cpuData;
  };*/
}

#endif
