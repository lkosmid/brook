// dx9.hpp
#pragma once

#include "../runtime.hpp"
#include "dx9base.hpp"

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

namespace brook {

  extern const char* DX9_RUNTIME_STRING;

  // TIM: 'helper' struct for defining the bounds
  // of a stream in its texture:
  struct DX9Rect
  {
    DX9Rect() {}
    DX9Rect( float inLeft, float inTop, float inRight, float inBottom )
      : left(inLeft), top(inTop), right(inRight), bottom(inBottom) {}
    
    operator float*() { return (float*)this; }
      
    operator const float*() const { return (const float*)this; }

    float left, top, right, bottom;
  };

  class DX9Kernel : public Kernel {
  public:
    DX9Kernel(DX9RunTime* runtime, const void* source[]);
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
    virtual ~DX9Kernel();
    IDirect3DDevice9* getDevice();
    void initialize( const char* source );

    int argumentStreamIndex;
    int argumentConstantIndex;
    int argumentOutputIndex;

    DX9RunTime* runtime;
    DX9PixelShader* pixelShader;
    DX9Rect inputRects[8]; // TIM: TODO: named constant?
    float4 inputConstants[8];
    IDirect3DTexture9* inputTextures[8];
    DX9Rect outputRect;
    IDirect3DSurface9* outputSurface;
  };

  class DX9Stream : public Stream {
  public:
    DX9Stream (DX9RunTime* runtime,const char type[], int dims, int extents[]);
    virtual void Read(const void* inData);
    virtual void Write(void* outData);
    virtual void Release() {}

    IDirect3DTexture9* getTextureHandle();
    IDirect3DSurface9* getSurfaceHandle();
    const DX9Rect& getInputRect() { return inputRect; }
    const DX9Rect& getOutputRect() { return outputRect; }

  private:
    virtual ~DX9Stream ();
    IDirect3DDevice9* getDevice();

    DX9RunTime* runtime;
    DX9Texture* texture;
    DX9Rect inputRect;
    DX9Rect outputRect;
  };

  class DX9RunTime : public RunTime {
  public:
    DX9RunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(const char type[], int dims, int extents[]);
    virtual ~DX9RunTime();

    IDirect3DDevice9* getDevice() { return device; }
    DX9VertexShader* getPassthroughVertexShader() {
      return passthroughVertexShader;
    }

    void execute( const DX9Rect& outputRect, const DX9Rect* inputRects );

  private:
    void initializeVertexBuffer();

    DX9Window* window;
    DX9VertexShader* passthroughVertexShader;
    IDirect3D9* direct3D;
    IDirect3DDevice9* device;
    IDirect3DVertexBuffer9* vertexBuffer;
    IDirect3DIndexBuffer9* indexBuffer;
    IDirect3DVertexDeclaration9* vertexDecl;
  };

  // more helper types used by the runtime
  typedef unsigned short UInt16;

  struct DX9Vertex
  {
    float4 position;
    float2 texcoords[8]; // TIM: TODO: named constant
  };
}
