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

namespace brook {
  class Kernel;
  class Stream;

  static const unsigned int MAXPROGLENGTH = 1024*32;
  static const unsigned int MAXSTREAMDIMS = 8;

  class Kernel {
  public:
    Kernel() {}
    virtual void PushStream(const Stream *s) = 0;
    virtual void PushConstant(const float &val) = 0;  
    virtual void PushConstant(const float2 &val) = 0;  
    virtual void PushConstant(const float3 &val) = 0; 
    virtual void PushConstant(const float4 &val) = 0;
    virtual void PushGatherStream(const Stream *s) = 0;
    virtual void PushOutput(const Stream *s) = 0;
    virtual void Map() = 0;
    virtual void Release() = 0;

  protected:
    virtual ~Kernel() {}
  };

  class Stream {
  public:
    Stream () {}
    virtual void Read(const void* inData) = 0;
    virtual void Write(void* outData) = 0;
    virtual void Release() = 0;
    enum USAGEFLAGS {NONE=0x0,READ=0x1,WRITE=0x2,READWRITE=0x3};
    virtual void * getData (unsigned int flags){return (void *)0;}
    virtual void releaseData(unsigned int flags){}
    virtual const unsigned int * getExtents() const {return (unsigned int*)0;}
    virtual unsigned int getDimension() const {return 0;}
    virtual unsigned int getTotalSize() const {return 0;}
  protected:
    virtual ~Stream() {}
  };
}


class __BRTStream {
public:
  __BRTStream(const char* type, ...);
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


#endif


