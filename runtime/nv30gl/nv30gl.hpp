#ifndef NV30GL_H
#define NV30GL_H

#include <windows.h>
#include "../runtime.hpp"

namespace brook {

  extern const char* NV30GL_RUNTIME_STRING;

  class NV30GLKernel : public Kernel {
  public:
    NV30GLKernel();
    virtual void PushStream(Stream *s);
    virtual void PushConstant(const float &val);  
    virtual void PushConstant(const float2 &val);  
    virtual void PushConstant(const float3 &val); 
    virtual void PushConstant(const float4 &val);
    virtual void PushReduce(void * val, __BRTStreamType type);
    virtual void PushGatherStream(Stream *s);
    virtual void PushOutput(Stream *s);
    virtual void Map();
    virtual void Release() {}
    
  private:
    virtual ~NV30GLKernel();
  };

  class NV30GLStream : public Stream {
  public:
    NV30GLStream (__BRTStreamType type, int dims, int extents[]);
    virtual void Read(const void* inData);
    virtual void Write(void* outData);
    virtual void Release() {}

  private:
    virtual ~NV30GLStream ();
  };
  class NV30GLIter : public Iter {
     int dims;
     int extents[2];
     float ranges[4];//maximum possible values for dx
  public:
     NV30GLIter(class NV30GLRunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[], 
             float ranges[]):Iter(type){
        if (dims>2)
           dims=2;//memory out of bounds check change to assert?
        this->dims=dims;
        for (int i=0;i<dims;++i) {
           this->extents[i]=extents[i];
        }
        unsigned int numranges=type*dims;
        if (numranges>4)
           numranges=4;//memory out of bounds check change to assert?
        memcpy(this->ranges,ranges,sizeof(float)*numranges);
     }
     virtual void * getData (unsigned int flags){assert(0);return 0;}
     virtual void releaseData(unsigned int flags){assert(0);0;}
     virtual const unsigned int * getExtents() const{assert(0);return 0;}
     virtual unsigned int getDimension() const {assert(0);return 0;}
     virtual __BRTStreamType getStreamType ()const{assert(0);return type;}
     virtual unsigned int getTotalSize() const {assert(0);return 0;}
  };

  class NV30GLRunTime : public RunTime {
  public:
    NV30GLRunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(__BRTStreamType type, int dims, int extents[]);
    virtual Iter* CreateIter(__BRTStreamType type, int dims, int e[],float r[]);
    virtual ~NV30GLRunTime();

  private: 
    HWND hwnd;
    HGLRC hglrc_window;
    HGLRC hglrc_workspace;
  };
}

#endif
