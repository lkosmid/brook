#ifndef NV30GL_H
#define NV30GL_H

#include <windows.h>
#include "../brt.hpp"

namespace brook {

  extern const char* NV30GL_RUNTIME_STRING;

  class NV30GLKernel : public Kernel {
  public:
    NV30GLKernel();
    virtual void PushStream(const Stream *s);
    virtual void PushConstant(const float &val);  
    virtual void PushConstant(const float2 &val);  
    virtual void PushConstant(const float3 &val); 
    virtual void PushConstant(const float4 &val);
    virtual void PushGatherStream(const Stream *s);
    virtual void PushOutput(const Stream *s);
    virtual void Map();
    virtual void Release() {}
    
  private:
    virtual ~NV30GLKernel();
  };

  class NV30GLStream : public Stream {
  public:
    NV30GLStream (const char type[], int dims, int extents[]);
    virtual void Read(const void* inData);
    virtual void Write(void* outData);
    virtual void Release() {}

  private:
    virtual ~NV30GLStream ();
  };

  class NV30GLRunTime : public RunTime {
  public:
    NV30GLRunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(const char type[], int dims, int extents[]);
    virtual ~NV30GLRunTime();

  private: 
    HWND hwnd;
    HGLRC hglrc_window;
    HGLRC hglrc_workspace;
  };
}

#endif
