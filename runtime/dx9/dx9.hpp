// dx9.hpp
#pragma once

#include "../runtime.hpp"
#include "dx9base.hpp"

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

namespace brook {

  extern const char* DX9_RUNTIME_STRING;

  enum {
    kDX9ReductionBufferWidth = 1024,
    kDX9ReductionBufferHeight = 1024
  };

  class DX9Kernel : public Kernel {
  public:
    DX9Kernel(DX9RunTime* runtime, const void* source[]);
    virtual void PushStream(Stream *s);
    virtual void PushIter(class Iter * v);
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
    void PushConstantImpl(const float4 &val);
    void ClearInputs();

    void ReduceToStream();
    void ReduceToValue();

    void ReduceDimension( int& ioReductionBufferSide,
      int inReductionTex0, int inReductionTex1,
      int inDimensionCount, int inDimensionToReduce,
      int inExtentToReduceTo, int* ioRemainingExtents );

    void ReduceDimensionToOne( int& ioReductionBufferSide,
      int inReductionTex0, int inReductionTex1,
      int inDimensionCount, int inDimensionToReduce, int* ioRemainingExtents );

    void BindReductionBaseState();
    void CopyStreamIntoReductionBuffer( DX9Stream* inStream );
    void BindReductionPassthroughState();
    void BindReductionOperationState();

    void DumpReductionBuffer( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax, int dim );
    void DumpReduceToOneState( int currentSide, int* slop, int core, int other, int dim );
    void DumpReduceDimensionState( int currentSide, int outputExtent,
      int remainingExtent, int remainingOtherExtent, int slopBufferCount, int dim );

    int argumentIndex;
    int argumentSamplerIndex;
    int argumentTexCoordIndex;
    int argumentConstantIndex;
    int argumentOutputIndex;
    int argumentReductionIndex;

    bool argumentUsesIndexof[32];

    DX9RunTime* runtime;
    DX9PixelShader* pixelShader;
    DX9Rect inputRects[8]; // TIM: TODO: named constant?
    float4 inputConstants[8];
    DX9Stream* inputStreams[8];
    IDirect3DTexture9* inputTextures[8];
    DX9Rect outputRect;
    DX9Stream* outputStream;
    IDirect3DSurface9* outputSurface;
    void* outputReductionData;
    __BRTStreamType outputReductionType;

    DX9Stream* inputReductionStream;
    int inputReductionStreamSamplerIndex;
    int inputReductionStreamTexCoordIndex;
    int outputReductionVarSamplerIndex;
    int outputReductionVarTexCoordIndex;
  };
  class DX9Iter : public Iter {
  public:
     DX9Iter(DX9RunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[], 
             float ranges[]);

     const DX9Rect& getRect() { return rect; }

     virtual void* getData (unsigned int flags);
     virtual void releaseData(unsigned int flags);
     virtual const unsigned int* getExtents() const { return extents; }
     virtual unsigned int getDimension() const { return dimensionCount; }
     virtual __BRTStreamType getStreamType () const { return elementType; }
     virtual unsigned int getTotalSize() const { return totalSize; }

  private:
     __BRTStreamType elementType;
     int componentCount;
     int dimensionCount;
     unsigned int extents[2];
     float ranges[8];
     int totalSize;
     DX9Rect rect;
     void* cpuBuffer;
     unsigned int cpuBufferSize;
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
    const float4& getIndexofConstant() { return indexofConstant; }
    int getWidth();
    int getHeight();
    DX9Rect getTextureSubRect( int l, int t, int r, int b );
    DX9Rect getSurfaceSubRect( int l, int t, int r, int b );

     virtual void* getData (unsigned int flags);
     virtual void releaseData(unsigned int flags);
     virtual const unsigned int* getExtents() const { return extents; }
     virtual unsigned int getDimension() const { return dimensionCount; }
     virtual __BRTStreamType getStreamType() const { return elementType; }
     virtual unsigned int getTotalSize() const { return totalSize; }

     void validateGPUData();
     void markGPUDataChanged();

  private:
    virtual ~DX9Stream ();
    IDirect3DDevice9* getDevice();

    int dimensionCount;
    unsigned int extents[8];
    __BRTStreamType elementType;
    unsigned int totalSize;
    int componentCount;

    DX9RunTime* runtime;
    DX9Texture* texture;
    DX9Rect inputRect;
    DX9Rect outputRect;
    float4 gatherConstant;
    float4 indexofConstant;
  };

  class DX9RunTime : public RunTime {
  public:
    DX9RunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(__BRTStreamType type, int dims, int extents[]);
    virtual Iter* CreateIter(__BRTStreamType type, 
                                 int dims, 
                                 int extents[],
                                 float range[]);
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
