// dx9.hpp
#pragma once

#include "../runtime.hpp"
#include "dx9base.hpp"

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

namespace brook {

  extern const char* DX9_RUNTIME_STRING;

  enum {
    kDX9MaximumTextureSize = 2048,
    kDX9ReductionBufferWidth = kDX9MaximumTextureSize,
    kDX9ReductionBufferHeight = kDX9MaximumTextureSize
  };

  class DX9Kernel : public Kernel {
  public:
    static DX9Kernel* create( DX9RunTime* inRuntime, const void* inSource[] );
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

    DX9Kernel( DX9RunTime* inRuntime );
    bool initialize( const void* inSource[] );
    bool initialize( const char* inSource );
    virtual ~DX9Kernel();

    void PushSampler( DX9Stream* s );
    void PushTexCoord( const DX9FatRect& r );
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
    IDirect3DDevice9* device;
    DX9PixelShader* pixelShader;
    DX9FatRect inputRects[8]; // TIM: TODO: named constant?
    float4 inputConstants[8];
    DX9Stream* inputStreams[8];
    IDirect3DTexture9* inputTextures[8];
    DX9FatRect outputRect;
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
     static DX9Iter* create( DX9RunTime* inRuntime, __BRTStreamType inElementType,
             int inDimensionCount, int* inExtents, float* inRanges );

     const DX9FatRect& getRect() { return rect; }

     virtual void* getData (unsigned int flags);
     virtual void releaseData(unsigned int flags);
     virtual const unsigned int* getExtents() const { return extents; }
     virtual unsigned int getDimension() const { return dimensionCount; }
     virtual unsigned int getTotalSize() const { return totalSize; }

  private:
     DX9Iter( DX9RunTime* inRuntime, __BRTStreamType inElementType );
     bool initialize( int inDimensionCount, int* inExtents, float* inRanges );

     int componentCount;
     int dimensionCount;
     unsigned int extents[2];
     float ranges[8];
     int totalSize;
     DX9FatRect rect;
     void* cpuBuffer;
     unsigned int cpuBufferSize;
  };
  class DX9Stream : public Stream {
  public:
    static DX9Stream* create( DX9RunTime* inRuntime, __BRTStreamType inElementType,
      int inDimensionCount, int* inExtents );
    virtual void Read(const void* inData);
    virtual void Write(void* outData);
    virtual void Release() {}

    IDirect3DTexture9* getTextureHandle();
    IDirect3DSurface9* getSurfaceHandle();
    const DX9FatRect& getInputRect() { return inputRect; }
    const DX9FatRect& getOutputRect() { return outputRect; }
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
     virtual unsigned int getTotalSize() const { return totalSize; }

     void validateGPUData();
     void markGPUDataChanged();

  private:
    DX9Stream( DX9RunTime* inRuntime, __BRTStreamType inElementType );
    bool initialize( int inDimensionCount, int* inExtents );
    virtual ~DX9Stream ();
    IDirect3DDevice9* getDevice();

    int dimensionCount;
    unsigned int extents[8];
    unsigned int totalSize;
    int componentCount;

    DX9RunTime* runtime;
    DX9Texture* texture;
    DX9FatRect inputRect;
    DX9FatRect outputRect;
    float4 gatherConstant;
    float4 indexofConstant;
  };

  class DX9RunTime : public RunTime {
  public:
    static DX9RunTime* create();

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

    void execute( const DX9FatRect& outputRect, const DX9FatRect* inputRects );

  private:
    DX9RunTime();
    bool initialize();

    void initializeVertexBuffer();

    DX9Window* window;
    DX9VertexShader* passthroughVertexShader;
    DX9PixelShader* passthroughPixelShader;
    DX9Texture* reductionBuffer;
    IDirect3D9* direct3D;
    IDirect3DDevice9* device;
    IDirect3DVertexBuffer9* vertexBuffer;
    IDirect3DVertexDeclaration9* vertexDecl;
  };

  // more helper types used by the runtime
  typedef unsigned short UInt16;

  struct DX9Vertex
  {
    float4 position;
    float4 texcoords[8]; // TIM: TODO: named constant
  };
}
