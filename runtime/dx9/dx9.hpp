#ifndef DX9_H
#define DX9_H

#include <brook.hpp>
#include "dx9base.hpp"

#define DX9_RUNTIME_STRING "dx9"

class DX9Kernel : public __BrookKernel {
public:
  DX9Kernel(DX9RunTime* runtime, const char* source[]);
  virtual void SetInput(const int arg, const __BrookStream *s);
  virtual void SetConstantFloat(const int arg, const float &val);  
  virtual void SetConstantFloat2(const int arg, const float2 &val);  
  virtual void SetConstantFloat3(const int arg, const float3 &val); 
  virtual void SetConstantFloat4(const int arg, const float4 &val);
  virtual void SetGatherInput(const int arg, const __BrookStream *s);
  virtual void SetOutput(const __BrookStream *s);
  virtual void Exec(void);
  virtual ~DX9Kernel();

private:
  IDirect3DDevice9* getDevice();
  void initialize( const char* source );
  int mapArgumentToTextureUnit( int arg );
  int mapArgumentToConstantIndex( int arg );

  DX9RunTime* runtime;
  DX9PixelShader* pixelShader;
};

class DX9Stream : public __BrookStream {
public:
  DX9Stream (DX9RunTime* runtime,const char type[], int dims, int extents[]);
  virtual void streamRead(void *p);
  virtual void streamWrite(void *p);
  virtual ~DX9Stream ();

  IDirect3DTexture9* getTextureHandle();
  IDirect3DSurface9* getSurfaceHandle();

private:
  IDirect3DDevice9* getDevice();

  DX9RunTime* runtime;
  DX9Texture* texture;
};

class DX9RunTime : public __BrookRunTime {
public:
  DX9RunTime();
  virtual __BrookKernel *LoadKernel(const char*[]);
  virtual __BrookStream *CreateStream(const char type[], int dims, int extents[]);
  virtual ~DX9RunTime();

  IDirect3DDevice9* getDevice() { return device; }
  DX9VertexShader* getPassthroughVertexShader() {
    return passthroughVertexShader;
  }

private:
  DX9Window* window;
  DX9VertexShader* passthroughVertexShader;
  IDirect3D9* direct3D;
  IDirect3DDevice9* device;
};



#endif
