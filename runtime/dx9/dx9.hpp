// dx9.hpp
#pragma once

#include "../runtime.hpp"
#include "dx9base.hpp"

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

namespace brook {

  extern const char* DX9_RUNTIME_STRING;

  class DX9Kernel : public Kernel {
  public:
    DX9Kernel(DX9RunTime* runtime, const void* source[]);
    virtual void PushStream(Stream *s);
    virtual void PushConstant(const float &val);  
    virtual void PushConstant(const float2 &val);  
    virtual void PushConstant(const float3 &val); 
    virtual void PushConstant(const float4 &val);
    virtual void PushGatherStream(Stream *s);
    virtual void PushReduce(void * input, __BRTStreamType);
    virtual void PushOutput(Stream *s);
    virtual void Map();
    virtual void Reduce();
    virtual void Release() {}

  private:
    enum {
      kBaseConstantIndex = 1
    };

    virtual ~DX9Kernel();
    IDirect3DDevice9* getDevice();
    void initialize( const char* source );

    void PushSampler( DX9Stream* s );
    void PushTexCoord( const DX9Rect& r );
    void ClearInputs();

    int argumentStreamIndex;
    int argumentSamplerIndex;
    int argumentTexCoordIndex;
    int argumentConstantIndex;
    int argumentOutputIndex;
    int argumentReductionIndex;

    DX9RunTime* runtime;
    DX9PixelShader* pixelShader;
    DX9Rect inputRects[8]; // TIM: TODO: named constant?
    float4 inputConstants[8];
    IDirect3DTexture9* inputTextures[8];
    DX9Stream* inputStreams[8];
    DX9Rect outputRect;
    IDirect3DSurface9* outputSurface;
    void* outputReductionData;
    __BRTStreamType outputReductionType;
  };

  class DX9Stream : public Stream {
  public:
    DX9Stream (DX9RunTime* runtime,__BRTStreamType type, int dims, int extents[]);
    virtual void Read(const void* inData);
    virtual void Write(void* outData);
    virtual void Release() {}

    IDirect3DTexture9* getTextureHandle();
    IDirect3DSurface9* getSurfaceHandle();
    const DX9Rect& getInputRect() { return inputRect; }
    const DX9Rect& getOutputRect() { return outputRect; }
    const float4& getGatherConstant() { return gatherConstant; }
    int getWidth();
    int getHeight();
    DX9Rect getTextureSubRect( int l, int t, int r, int b );
    DX9Rect getSurfaceSubRect( int l, int t, int r, int b );

  private:
    virtual ~DX9Stream ();
    IDirect3DDevice9* getDevice();

    DX9RunTime* runtime;
    DX9Texture* texture;
    DX9Rect inputRect;
    DX9Rect outputRect;
    float4 gatherConstant;
  };

  class DX9RunTime : public RunTime {
  public:
    DX9RunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(__BRTStreamType type, int dims, int extents[]);
    virtual ~DX9RunTime();

    IDirect3DDevice9* getDevice() { return device; }
    DX9VertexShader* getPassthroughVertexShader() {
      return passthroughVertexShader;
    }
    DX9PixelShader* getPassthroughPixelShader() {
      return passthroughPixelShader;
    }
    DX9Texture* getReductionBuffer();

    void execute( const DX9Rect& outputRect, const DX9Rect* inputRects );

  private:
    static const int kReductionBufferSize = 1024;

    void initializeVertexBuffer();

    DX9Window* window;
    DX9VertexShader* passthroughVertexShader;
    DX9PixelShader* passthroughPixelShader;
    DX9Texture* reductionBuffer;
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
