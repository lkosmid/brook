// dx9.hpp
#pragma once

#include "../runtime.hpp"
#include "dx9base.hpp"

#include <vector>

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
    virtual void PushReduce(void * input, __BRTStreamType type);
    virtual void PushOutput(Stream *s);
    virtual void Map();
    virtual void Reduce();

  private:
    DX9Kernel( DX9RunTime* inRuntime );
    bool initialize( const void* inSource[] );
    bool initialize( const ::brook::desc::gpu_kernel_desc* inDescriptor );
    virtual ~DX9Kernel();

    void pushArgument();
    void pushStreamArgument( DX9Stream* inStream );
    void pushConstantImpl( const float4& inValue );
    void pushATShapeConstant( DX9Stream* inStream ); // special-case hack?
    void pushSamplers( DX9Stream* inStream );
    void pushSampler( IDirect3DBaseTexture9* inSampler );
    void pushInterpolant( const DX9FatRect& inRect );
    void pushOutputs( DX9Stream* inStream );
    void pushOutput( IDirect3DSurface9* inOutput );

    void clearArguments();
    void clearInputs();
//    void PushSamplers( DX9Stream* s );
//    void PushTexCoord( const DX9FatRect& r );
//    int PushConstantImpl(const float4 &val);
//    void ClearInputs();

    void ReduceToStream( DX9Texture* inOutputBuffer );

    class ReductionState
    {
    public:
      DX9Texture* inputTexture; // the texture we are reducing from
      DX9Texture* outputTexture; // the texture we are reducing to

      int whichBuffer; // -1 for inputTexture, 0,1 for reductionBuffers
      DX9Texture* reductionBuffers[2];
      DX9Texture* slopBuffer;

      int currentDimension; // 0 for X, 1 for Y

      // the sizes of the various buffers
      int targetExtents[2];
      int inputExtents[2];
      int currentExtents[2];
      int slopCount;
    };

    void executeReductionTechnique( int inFactor );
    void beginReduction( ReductionState& ioState );
    void executeReductionStep( ReductionState& ioState );
    void executeSlopStep( ReductionState& ioState );
    void endReduction( ReductionState& ioState );
    void dumpReductionState( ReductionState& ioState );
    void dumpReductionBuffer( DX9Texture* inBuffer, int inWidth, int inHeight );

    std::vector<DX9Stream*> argumentStreams;
    std::vector<DX9Stream*> outputStreams;
    std::vector<float4> argumentConstants;
    std::vector<IDirect3DBaseTexture9*> argumentSamplers;
    std::vector<DX9FatRect> argumentInterpolants;
    std::vector<IDirect3DSurface9*> argumentOutputs;

    struct ReductionArgumentInfo
    {
      ReductionArgumentInfo( void* inData, __BRTStreamType inType )
        : data(inData), type(inType) {}

      void* data;
      __BRTStreamType type;
    };

    std::vector<ReductionArgumentInfo> argumentReductions;
    size_t inputReductionArgumentIndex;
    size_t outputRedctionArgumentIndex;

    std::vector<float4> globalConstants;
    std::vector<IDirect3DBaseTexture9*> globalSamplers;
    std::vector<IDirect3DSurface9*> globalOutputs;
    std::vector<DX9FatRect> globalInterpolants;

    // special magic
    std::vector<size_t> atShapeConstants;

/* TIM: complete rewrite...
    int argumentIndex;

    std::vector<bool> argumentUsesIndexof;
    bool hasPushedOutputIndexof;

    std::vector<DX9Stream*> outputStreams;
    std::vector<IDirect3DSurface9*> outputSurfaces;
    int outputWidth, outputHeight;

    std::vector<float4> inputConstants;

    std::vector<DX9Stream*> inputStreams;
    std::vector<int> inputStreamShapeConstantIndices;
    std::vector<IDirect3DTexture9*> inputTextures;

    std::vector<void*> outputReductionDatas;
    std::vector<__BRTStreamType> outputReductionTypes;
*/
    std::vector<DX9FatRect> inputTextureRects;
    DX9FatRect outputRect;

    DX9RunTime* runtime;
    IDirect3DDevice9* device;
//    DX9PixelShader* pixelShader;
/*    class Pass
    {
    public:
      DX9PixelShader* pixelShader;
      int firstOutput, outputCount;
    };
    std::vector<Pass> standardPasses;
    std::vector<Pass> fullTranslationPasses;*/
    DX9Stream* mustMatchShapeStream;
    void matchStreamShape( DX9Stream* inStream );
    bool streamShapeMismatch;
/*
    class ReductionPass
    {
    public:
      int reductionFactor;
      std::vector<Pass> standardPasses;
      std::vector<Pass> fullTranslationPasses;
    };

    std::vector<ReductionPass> reductionPasses;
    void addReductionPass( int inReductionFactor, bool inFullTranslation, const Pass& inPass );
*/

    // TIM: new technique/pass holders
    struct Input
    {
      Input( const ::brook::desc::gpu_input_desc& input )
        : argumentIndex(input._argumentIndex), componentIndex(input._componentIndex)
      {}

      int argumentIndex;
      int componentIndex;
    };

    struct Pass
    {
      DX9PixelShader* pixelShader;

      void addConstant( const ::brook::desc::gpu_input_desc& input ) {
        constants.push_back( Input(input) );
      }

      void addSampler( const ::brook::desc::gpu_input_desc& input ) {
        samplers.push_back( Input(input) );
      }

      void addInterpolant( const ::brook::desc::gpu_input_desc& input ) {
        interpolants.push_back( Input(input) );
      }

      void addOutput( const ::brook::desc::gpu_input_desc& input ) {
        outputs.push_back( Input(input) );
      }

      std::vector<Input> constants;
      std::vector<Input> samplers;
      std::vector<Input> interpolants;
      std::vector<Input> outputs;
    };

    struct Technique
    {
      int reductionFactor;
      bool outputAddressTranslation;
      bool inputAddressTranslation;
      std::vector<Pass> passes;  
    };

    struct ArgumentInfo
    {
      int firstConstantIndex; int constantCount;
      int firstSamplerIndex; int samplerCount;
      int firstInterpolantIndex; int interpolantCount;
      int firstOutputIndex; int outputCount;
    };

    std::vector<Technique> techniques;

    void mapTechnique( const Technique& inTechnique );
    void mapPass( const Pass& inPass );

    void bindConstant( size_t inConstantIndex, const Input& inInput );
    void bindSampler( size_t inSamplerIndex, const Input& inInput );
    void bindInterpolant( size_t inInterpolantIndex, const Input& inInput );
    void bindOutput( size_t inOutputIndex, const Input& inInput );
    void disableOutput( size_t inOutputIndex );

    float4 getConstant( int inArgument, int inComponent );
    IDirect3DBaseTexture9* getSampler( int inArgument, int inComponent );
    DX9FatRect getInterpolant( int inArgument, int inComponent );
    IDirect3DSurface9* getOutput( int inArgument, int inComponent );
    ArgumentInfo& getCurrentArgument();
    size_t getCurrentArgumentIndex();

    std::vector<ArgumentInfo> arguments;

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
    static DX9Stream* create( DX9RunTime* inRuntime,
      int inFieldCount, const __BRTStreamType* inFieldTypes,
      int inDimensionCount, const int* inExtents );
    virtual void Read(const void* inData);
    virtual void Write(void* outData);

//    DX9Texture* getTexture() { return texture; }
//    IDirect3DTexture9* getTextureHandle();
//    IDirect3DSurface9* getSurfaceHandle();

    int getSubstreamCount();
    DX9Texture* getIndexedTexture( int inIndex ) const;
    IDirect3DTexture9* getIndexedTextureHandle( int inIndex ) const;
    IDirect3DSurface9* getIndexedSurfaceHandle( int inIndex ) const;

    const DX9FatRect& getInputRect() { return inputRect; }
    const DX9FatRect& getOutputRect() { return outputRect; }
    const float4& getGatherConstant() { return gatherConstant; }
    const float4& getIndexofConstant() { return indexofConstant; }
    int getTextureWidth();
    int getTextureHeight();
    DX9Rect getTextureSubRect( int l, int t, int r, int b );
    DX9Rect getSurfaceSubRect( int l, int t, int r, int b );

    float4 getATOutputConstant();
    float4 getATOutputShape();
    float4 getATShapeConstant( const float4& outputShape );
    float4 getATLinearizeConstant();
    float4 getATReshapeConstant();
    float4 getATInverseShapeConstant();
    DX9Rect getATAddressInterpolantRect();

     virtual void* getData (unsigned int flags);
     virtual void releaseData(unsigned int flags);
     virtual const unsigned int* getExtents() const { return &extents[0]; }
     virtual unsigned int getDimension() const { return dimensionCount; }
     virtual unsigned int getTotalSize() const { return totalSize; }
     virtual int getFieldCount() const { return (int)fields.size(); }
     virtual __BRTStreamType getIndexedFieldType(int i) const {
       return fields[i].elementType;
     }

     virtual void* getIndexedFieldRenderData(int i) const {
       return (void*)getIndexedTextureHandle(i);
     }
     virtual void synchronizeRenderData() {
       validateGPUData();
     }

     void validateGPUData();
     void markGPUDataChanged();



  private:
    DX9Stream( DX9RunTime* inRuntime );
    bool initialize(
      int inFieldCount, const __BRTStreamType* inFieldTypes,
      int inDimensionCount, const int* inExtents );
    virtual ~DX9Stream ();
    IDirect3DDevice9* getDevice();
     void ReadImpl(const void* inData);
     void WriteImpl(void* outData);

    class Field
    {
    public:
      Field( __BRTStreamType inElementType, int inComponentCount, DX9Texture* inTexture )
        : elementType(inElementType), componentCount(inComponentCount), texture(inTexture)
      {}

      __BRTStreamType elementType;
      int componentCount;
      DX9Texture* texture;
    };

    unsigned int dimensionCount;
    unsigned int totalSize;
    std::vector<unsigned int> extents;
    std::vector<unsigned int> reversedExtents;
    std::vector<Field> fields;

    int textureWidth, textureHeight;

    void validateSystemData();
    void markSystemDataChanged();

    char* systemDataBuffer;
    unsigned int systemDataBufferSize;
    bool systemDataChanged;
    bool gpuDataChanged;

    DX9RunTime* runtime;
    DX9FatRect inputRect;
    DX9FatRect outputRect;
    float4 gatherConstant;
    float4 indexofConstant;
  };

  class DX9RunTime : public RunTime {
  public:
    static DX9RunTime* create( bool inAddressTranslation, void* inContextValue = 0 );

    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]);
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
    DX9Texture* getReductionBuffer0( int inMinWidth, int inMinHeight );
    DX9Texture* getReductionBuffer1( int inMinWidth, int inMinHeight );
    DX9Texture* getSlopBuffer( int inMinWidth, int inMinHeight );
    DX9Texture* getReductionTargetBuffer();

    void execute( const DX9FatRect& outputRect, int inputRectCount, const DX9FatRect* inputRects );

    bool isAddressTranslationOn() { return addressTranslation; }

  private:
    DX9RunTime( bool addressTranslation );
    bool initialize( void* inContextValue );

    void initializeVertexBuffer();

    bool addressTranslation;
    DX9Window* window;
    DX9VertexShader* passthroughVertexShader;
    DX9PixelShader* passthroughPixelShader;
    DX9Texture* reductionBuffer0;
    DX9Texture* reductionBuffer1;
    DX9Texture* slopBuffer;
    DX9Texture* reductionTargetBuffer;
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
