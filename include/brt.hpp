#ifndef BRT_HPP
#define BRT_HPP

//vector is needed by the output of the compilation
typedef struct {
  float x,y;
} float2;

typedef struct {
  float x,y,z;
} float3;

typedef struct {
  float x,y,z,w;
} float4;
enum __BRTStreamType {
   __BRTSTREAM=0,//stream of stream is illegal. Used in reduce to stream.
   __BRTFLOAT=1,
   __BRTFLOAT2=2,
   __BRTFLOAT3=3,
   __BRTFLOAT4=4,
};

namespace brook {
  class Kernel;
  class Stream;

  static const unsigned int MAXPROGLENGTH = 1024*32;
  static const unsigned int MAXSTREAMDIMS = 8;

  class Kernel {
  public:
    Kernel() {}
    virtual void PushStream(Stream *s) = 0;
    virtual void PushConstant(const float &val) = 0;  
    virtual void PushConstant(const float2 &val) = 0;  
    virtual void PushConstant(const float3 &val) = 0; 
    virtual void PushConstant(const float4 &val) = 0;
    virtual void PushReduce (void * val,  __BRTStreamType type)=0;
    virtual void PushGatherStream(Stream *s) = 0;
    virtual void PushOutput(Stream *s) = 0;
    virtual void Map() = 0;
    virtual void Reduce() {Map();}
    virtual void Release() = 0;

  protected:
    virtual ~Kernel() {}
  };
  class Stream {
  public:
    Stream (__BRTStreamType type) {this->type=type;}
    virtual void Read(const void* inData) = 0;
    virtual void Write(void* outData) = 0;
    virtual void Release() = 0;
    enum USAGEFLAGS {NONE=0x0,READ=0x1,WRITE=0x2,READWRITE=0x3};
    virtual void * getData (unsigned int flags){return (void *)0;}
    virtual void releaseData(unsigned int flags){}
    virtual const unsigned int * getExtents() const {return (unsigned int*)0;}
    virtual unsigned int getDimension() const {return 0;}
    virtual unsigned int getTotalSize() const {return 0;}
    virtual unsigned int getStride() const {return sizeof(float)*getStreamType();}
    virtual __BRTStreamType getStreamType ()const{return type;}
  protected:
    __BRTStreamType type;
    virtual ~Stream() {}
  };
}


class __BRTStream {
public:
  __BRTStream(__BRTStreamType , ...);
  ~__BRTStream()
  {
    if( stream != 0 )
      stream->Release();
  }

  operator brook::Stream*() const {
    return stream;
  }

  brook::Stream* operator->() const {
    return stream;
  }

private:
  __BRTStream( const __BRTStream& ); // no copy constructor
  brook::Stream* stream;
};

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

inline static void streamRead( brook::Stream *s, void *p) {
  s->Read(p);
}

inline static void streamWrite( brook::Stream *s, void *p) {
  s->Write(p);
}

template <class T> inline __BRTStreamType __BRTReductionType(const T*e) {
   const float * f = e;
   //error case! will complain about casting an T to a float.
   return __BRTFLOAT;
}
template <> inline __BRTStreamType __BRTReductionType(const __BRTStream*e) {
   return __BRTSTREAM;
}
template <> inline __BRTStreamType __BRTReductionType(const float4*e) {
   return __BRTFLOAT4;
}
template <> inline __BRTStreamType __BRTReductionType(const float3*e) {
   return __BRTFLOAT3;
}
template <> inline __BRTStreamType __BRTReductionType(const float2*e) {
   return __BRTFLOAT2;
}
template <> inline __BRTStreamType __BRTReductionType(const float*e) {
   return __BRTFLOAT;
}
inline unsigned int getIndexOf(unsigned int i, 
                               const unsigned int *extent,
                               unsigned int dim,
                               const unsigned int *refextent) {
   return i;
}



#endif


