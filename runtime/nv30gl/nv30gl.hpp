#ifndef NV30GL_H
#define NV30GL_H

#include <brook.hpp>

#include <windows.h>

#define NV30GL_RUNTIME_STRING "nv30gl"

class NV30GLKernel : public __BrookKernel {
public:
  NV30GLKernel();
  virtual void SetInput(const int arg, const __BrookStream *s);
  virtual void SetConstantFloat(const int arg, const float &val);  
  virtual void SetConstantFloat2(const int arg, const float2 &val);  
  virtual void SetConstantFloat3(const int arg, const float3 &val); 
  virtual void SetConstantFloat4(const int arg, const float4 &val);
  virtual void SetGatherInput(const int arg, const __BrookStream *s);
  virtual void SetOutput(const __BrookStream *s);
  virtual void Exec(void);
  virtual ~NV30GLKernel();
};

class NV30GLStream : public __BrookStream {
public:
  NV30GLStream (const char type[], int dims, int extents[]);
  virtual void streamRead(void *p);
  virtual void streamWrite(void *p);
  virtual ~NV30GLStream ();
};

class NV30GLRunTime : public __BrookRunTime {
public:
  NV30GLRunTime();
  virtual __BrookKernel *LoadKernel(const char*[]);
  virtual __BrookStream *CreateStream(const char type[], int dims, int extents[]);
  virtual ~NV30GLRunTime();

private: 
  HWND hwnd;
};

#endif
