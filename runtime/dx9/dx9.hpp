#ifndef DX9_H
#define DX9_H

#include <brook.hpp>

#define DX9_RUNTIME_STRING "dx9"

class DX9Kernel : public __BrookKernel {
public:
  DX9Kernel();
  virtual void SetInput(const int arg, const __BrookStream *s);
  virtual void SetConstantFloat(const int arg, const float &val);  
  virtual void SetConstantFloat2(const int arg, const float2 &val);  
  virtual void SetConstantFloat3(const int arg, const float3 &val); 
  virtual void SetConstantFloat4(const int arg, const float4 &val);
  virtual void SetGatherInput(const int arg, const __BrookStream *s);
  virtual void SetOutput(const __BrookStream *s);
  virtual void Exec(void);
  virtual ~DX9Kernel();
};

class DX9Stream : public __BrookStream {
public:
  DX9Stream (const char type[], const __BrookIntArray &);
  virtual void streamRead(void *p);
  virtual void streamWrite(void *p);
  virtual ~DX9Stream ();
};

class DX9RunTime : public __BrookRunTime {
public:
  DX9RunTime();
  virtual __BrookKernel *LoadKernel(const char[][__MAXPROGLENGTH]);
  virtual __BrookStream *CreateStream(char type[], int ndims, ...);
  virtual ~DX9RunTime();
};



#endif
