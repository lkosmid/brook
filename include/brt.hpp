#ifndef BRT_HPP
#define BRT_HPP
#ifndef _WIN32
#include <stdlib.h>
#endif
extern "C" {
#include <assert.h>
};

//vector is needed by the output of the compilation
typedef struct float2 {
  float2(float _x, float _y) { x = _x; y = _y; }
  float2(void) {}

  float x,y;
} float2;

typedef struct float3 {
  float3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
  float3(void) {}

  float x,y,z;
} float3;

typedef struct float4 {
  float4(float _x, float _y, float _z, float _w) {
     x = _x; y = _y; z = _z; w = _w;
  }
  float4(void) {}

  float x,y,z,w;
} float4;
enum __BRTStreamType {
   __BRTNONE=-1,
   __BRTSTREAM=0,//stream of stream is illegal. Used in reduce to stream.
   __BRTFLOAT=1,
   __BRTFLOAT2=2,
   __BRTFLOAT3=3,
   __BRTFLOAT4=4,
};
float getSentinel();

namespace brook {
  class Kernel;
  class StreamInterface;
  class Stream;
  class Iter;
  class stream;
  class iter;

  static const unsigned int MAXPROGLENGTH = 1024*32;
  static const unsigned int MAXSTREAMDIMS = 8;

  void initialize( const char* inRuntimeName, void* inContextValue = 0 );

    class StreamInterface {
  private:
    size_t referenceCount;
  protected:
    StreamInterface()
      : referenceCount(1) {}
    virtual ~StreamInterface(){}
  public:
    void acquireReference() { referenceCount++; }
    void releaseReference() {
      if( --referenceCount == 0 ) delete this;
    }

    enum USAGEFLAGS {NONE=0x0,READ=0x1,WRITE=0x2,READWRITE=0x3};
    virtual void * getData (unsigned int flags)=0;
    virtual void releaseData(unsigned int flags)=0;
    virtual void readItem(void * p,unsigned int * index);
    virtual const unsigned int * getExtents() const=0;
    virtual unsigned int getDimension() const {return 0;}

    unsigned int getElementSize() const;
    virtual int getFieldCount() const = 0;
    virtual __BRTStreamType getIndexedFieldType(int i) const=0;

    // functions for getting at low-level representation,
    // so that an application can render and simulate
    virtual void* getIndexedFieldRenderData(int i) const { return 0; }
    virtual void synchronizeRenderData() {}

    //virtual __BRTStreamType getStreamType ()const=0;
    virtual unsigned int getTotalSize() const {
       unsigned int ret=1;
       unsigned int dim=getDimension();
       const unsigned int * extents = getExtents();
       for (unsigned int i=0;i<dim;++i) {
          ret*=extents[i];
       }
       return ret;
    }
  };

  class Stream : public StreamInterface {
  public:
    Stream () {}
    virtual void Read(const void* inData) = 0;
    virtual void Write(void* outData) = 0;
    
    //virtual unsigned int getStride() const {return sizeof(float)*getStreamType();}
    //virtual __BRTStreamType getStreamType ()const{return type;}
  protected:
    virtual ~Stream() {}
  };

  class Iter : public StreamInterface {
  public:
    Iter (__BRTStreamType type) {this->type=type;}
    virtual int getFieldCount() const { return 1; }
    virtual __BRTStreamType getIndexedFieldType(int i) const {
      assert(i == 0);
      return type;
    }

//    virtual __BRTStreamType getStreamType ()const{return type;}
  protected:
    __BRTStreamType type;
    virtual ~Iter() {}
  };

  class stream
  {
  public:
    stream();
    stream( const stream& );
    stream& operator=( const stream& );

    // easy-to-use constructors for C++ interface
    template<typename T>
    static stream create( int x ) {
      return stream( ::brook::getStreamType((T*)0), inExtent0, -1 );
    }

    template<typename T>
    static stream create( int y, int x ) {
      return stream( ::brook::getStreamType((T*)0), y, x, -1 );
    }

    // standard constructors for BRCC-generated code
    stream(const __BRTStreamType*,...);
    stream(int * extents,int dims,const __BRTStreamType *type);
    stream( const ::brook::iter& );
    ~stream();

    void swap(::brook::stream& other) {
      ::brook::Stream* s = other._stream;
      other._stream = _stream;
      _stream = s;
    }

    operator ::brook::Stream*() const {
      return _stream;
    }


    operator ::brook::StreamInterface*() const {
      return _stream;
    }

    ::brook::Stream* operator->() const {
      return _stream;
    }

  private:
    ::brook::Stream* _stream;
  };

  class Kernel {
  public:
    Kernel() {}
    virtual void PushStream(Stream *s) = 0;
    virtual void PushIter(class Iter * v) = 0;
    virtual void PushConstant(const float &val) = 0;  
    virtual void PushConstant(const float2 &val) = 0;  
    virtual void PushConstant(const float3 &val) = 0; 
    virtual void PushConstant(const float4 &val) = 0;
    virtual void PushReduce (void * val,  __BRTStreamType type)=0;
    virtual void PushGatherStream(Stream *s) = 0;
    virtual void PushOutput(Stream *s) = 0;
    virtual void Map() = 0;
    virtual void Reduce() {Map();}
    virtual void Release() {delete this;}

  protected:
    virtual ~Kernel() {}
  };

  template<typename T>
  const __BRTStreamType* getStreamType(T* unused=0);

  template<>
  inline const __BRTStreamType* getStreamType(float*) {
    static const __BRTStreamType result[] = {__BRTFLOAT,__BRTNONE};
    return result;
  }

  template<>
  inline const __BRTStreamType* getStreamType(float2*) {
    static const __BRTStreamType result[] = {__BRTFLOAT2,__BRTNONE};
    return result;
  }

  template<>
  inline const __BRTStreamType* getStreamType(float3*) {
    static const __BRTStreamType result[] = {__BRTFLOAT3,__BRTNONE};
    return result;
  }

  template<>
  inline const __BRTStreamType* getStreamType(float4*) {
    static const __BRTStreamType result[] = {__BRTFLOAT4,__BRTNONE};
    return result;
  }

  class iter {
  public:
    iter(__BRTStreamType,...);
    ~iter() {
      if(_iter) _iter->releaseReference();
    }

    operator ::brook::Iter*() const {
      return _iter;
    }

    operator ::brook::StreamInterface*() const {
      return _iter;
    }

    ::brook::Iter* operator->() const {
      return _iter;
    }

  private:
    iter( const ::brook::iter& ); // no copy constructor
    ::brook::Iter* _iter;
  };

  class RunTime;
  RunTime* createRunTime( bool useAddressTranslation );
}

// TIM: legacy support?
typedef ::brook::stream __BRTStream;
typedef ::brook::iter __BRTIter;

class __BRTKernel {
public:
  __BRTKernel(const void* code[]);
  ~__BRTKernel()
  {
    if( kernel != 0 )
      kernel->Release();
  }

  operator brook::Kernel*() const {
    return kernel;
  }

  brook::Kernel* operator->() const {
    return kernel;
  }

private:
  __BRTKernel( const __BRTKernel& ); // no copy constructor
  brook::Kernel* kernel;
};
inline static void maxDimension(int * out, 
                                const unsigned int * in,
                                int dims) {
   for (int i=0;i<dims;++i) {
      if (in[i]>(unsigned int)out[i])out[i]=in[i];
   }
}
__BRTStream* sentinelStream(int dim);
inline static void streamRead( brook::Stream *s, void *p) {
  s->Read(p);
}
void streamPrint(brook::StreamInterface*s, bool flatten=false);
inline static void streamWrite( brook::Stream *s, void *p) {
  s->Write(p);
}

void readItem(brook::StreamInterface *s, void * p, ... );
inline static float4 streamSize(::brook::stream &s) {
   float4 ret(0.0f,0.0f,0.0f,0.0f);
   const unsigned int * extents = s->getExtents();
   unsigned int dim = s->getDimension();
   switch (s->getDimension()) {
   case 3:
      ret.z=(float)extents[dim-3];
   case 2:
      ret.y=(float)extents[dim-2];
   case 1:
      ret.x=(float)extents[dim-1];
      break;
   case 4:
   default:{
      for (unsigned int i=0;i<dim-3;++i) ret.w+=(float)extents[i];
      ret.z=(float)extents[dim-3];
      ret.y=(float)extents[dim-2];
      ret.x=(float)extents[dim-1];
      break;
   }
   }
   return ret;
}
inline static void streamSwap(::brook::stream &x, ::brook::stream &y) {
   x.swap(y);
}
#endif
