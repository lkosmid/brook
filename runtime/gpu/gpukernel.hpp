// gpukernel.hpp
#ifndef GPU_KERNEL_HPP
#define GPU_KERNEL_HPP

#include "gpubase.hpp"
#include "gpucontext.hpp"

namespace brook {

  class GPURuntime;

  class GPUKernel : public Kernel {
  public:
    typedef GPUContext::TextureHandle TextureHandle;
    typedef GPUContext::TextureHandle TextureHandle;
    typedef GPUContext::PixelShaderHandle PixelShaderHandle;
    typedef GPUContext::VertexShaderHandle VertexShaderHandle;

    static GPUKernel* create( GPURuntime* inRuntime, const void* inSource[] );

    // BRT Interface
    virtual void PushStream( Stream* inStream );
    virtual void PushIter( Iter* inIterator );
    virtual void PushConstant( const float& inValue );  
    virtual void PushConstant( const float2& inValue );  
    virtual void PushConstant( const float3& inValue ); 
    virtual void PushConstant( const float4& inValue );
    virtual void PushGatherStream( Stream* inStream );
    virtual void PushReduce( void* outValue, __BRTStreamType inType );
    virtual void PushOutput(Stream *s);
    virtual void Map();
    virtual void Reduce();

  private:
    GPURuntime* _runtime;
    GPUContext* _context;

    GPUKernel( GPURuntime* inRuntime );
    bool initialize( const void* inSource[] );
    bool initialize( const ::brook::desc::gpu_kernel_desc* inDescriptor );
    virtual ~GPUKernel();

    class ArgumentType
    {
    public:
      virtual TextureHandle getTexture( GPUKernel* inKernel, 
                                        size_t inIndex, 
                                        size_t inComponent );
      virtual void getInterpolant( GPUKernel* inKernel, 
                                   size_t inIndex, 
                                   size_t inComponent,
                                   GPUInterpolant &outInterpolant);
      virtual float4 getConstant( GPUKernel* inKernel, 
                                  size_t inIndex, 
                                  size_t inComponent );
    };

    class StreamArgumentType : public ArgumentType
    {
    public:
      TextureHandle getTexture( GPUKernel* inKernel, 
                                size_t inIndex, 
                                size_t inComponent );
      void getInterpolant( GPUKernel* inKernel, 
                           size_t inIndex, 
                           size_t inComponent,
                           GPUInterpolant &outInterpolant );
      float4 getConstant( GPUKernel* inKernel, 
                          size_t inIndex, 
                          size_t inComponent );
    };

    friend class StreamArgumentType;//internal classes are not friendly enough
    class IteratorArgumentType : public ArgumentType
    {
    public:
      void getInterpolant( GPUKernel* inKernel, 
                           size_t inIndex, 
                           size_t inComponent,
                           GPUInterpolant &outInterpolant );
      float4 getConstant( GPUKernel* inKernel, 
                          size_t inIndex, 
                          size_t inComponent );
    };

    friend class IteratorArgumentType; //internal classes are not friends
    class ConstantArgumentType : public ArgumentType
    {
    public:
      float4 getConstant( GPUKernel* inKernel, 
                          size_t inIndex, 
                          size_t inComponent );
    };
    friend class ConstantArgumentType; //internal classes are not friends

    class GatherArgumentType : public ArgumentType
    {
    public:
      TextureHandle getTexture( GPUKernel* inKernel, 
                                size_t inIndex, 
                                size_t inComponent );
      float4 getConstant( GPUKernel* inKernel, 
                          size_t inIndex, 
                          size_t inComponent );
    };
    friend class GatherArgumentType; //internal classes are not friends

    class OutputArgumentType : public ArgumentType
    {
    public:
      TextureHandle getTexture( GPUKernel* inKernel, 
                                size_t inIndex, 
                                size_t inComponent );
      void getInterpolant( GPUKernel* inKernel, 
                           size_t inIndex, 
                           size_t inComponent,
                           GPUInterpolant &outInterpolant);
      float4 getConstant( GPUKernel* inKernel,
                          size_t inIndex,
                          size_t inComponent );
    };
    friend class OutputArgumentType; //internal classes are not friends

    class ArgumentInfo
    {
    public:
      ArgumentInfo( ArgumentType* inType, 
                    size_t inIndex )
        : type(inType), index(inIndex)
      {}

      TextureHandle getTexture( GPUKernel* inKernel, 
                                size_t inComponent ) {
        return type->getTexture( inKernel, index, inComponent );
      }

      void getInterpolant( GPUKernel* inKernel, 
                           size_t inComponent,
                           GPUInterpolant &outInterpolant) {
        type->getInterpolant( inKernel, index, inComponent, 
                              outInterpolant );
      }

      float4 getConstant( GPUKernel* inKernel, 
                          size_t inComponent ) {
        return type->getConstant( inKernel, index, inComponent );
      }

      ArgumentType* type;
      size_t index;
    };

    class Input
    {
    public:
      Input()
      {
      }

      Input( const ::brook::desc::gpu_input_desc& inDescriptor )
        : argumentIndex(inDescriptor._argumentIndex), 
          componentIndex(inDescriptor._componentIndex)
      {
      }

      size_t argumentIndex;
      size_t componentIndex;
    };
    typedef std::vector<Input> InputList;

    class Pass
    {
    public:
      void addConstant( const ::brook::desc::gpu_input_desc& inDescriptor ) {
        constants.push_back( Input(inDescriptor) );
      }

      void addSampler( const ::brook::desc::gpu_input_desc& inDescriptor ) {
        samplers.push_back( Input(inDescriptor) );
      }

      void addInterpolant( const ::brook::desc::gpu_input_desc& inDescriptor ) {
        interpolants.push_back( Input(inDescriptor) );
      }

      void addOutput( const ::brook::desc::gpu_input_desc& inDescriptor ) {
        outputs.push_back( Input(inDescriptor) );
      }

      PixelShaderHandle pixelShader;

      InputList constants;
      InputList samplers;
      InputList interpolants;
      InputList outputs;
    };
    typedef std::vector<Pass> PassList;

    class Technique
    {
    public:
      int reductionFactor;
      bool outputAddressTranslation;
      bool inputAddressTranslation;

      PassList passes;
    };

    // internal methods
    void pushArgument( ArgumentType* inType, size_t inIndex );
    void clearArguments();
    void clearInputs();
    void executeTechnique( const Technique& inTechnique );
    void executePass( const Pass& inPass );
    void bindConstant( size_t inIndex, const Input& inInput );
    void bindSampler( size_t inIndex, const Input& inInput );
    void bindInterpolant( size_t inIndex, const Input& inInput );
    void bindOutput( size_t inIndex, const Input& inInput );

    // shared data
    static ArgumentType* sStreamArgumentType;
    static ArgumentType* sIteratorArgumentType;
    static ArgumentType* sConstantArgumentType;
    static ArgumentType* sGatherArgumentType;
    static ArgumentType* sOutputArgumentType;

    // instance data
    std::vector<Technique> _techniques;

    typedef std::vector<GPUStream*> StreamList;
    typedef std::vector<GPUIterator*> IteratorList;
    typedef std::vector<float4> ConstantList;
    typedef std::vector<ArgumentInfo> ArgumentList;

    StreamList _streamArguments;
    IteratorList _iteratorArguments;
    ConstantList _constantArguments;
    StreamList _gatherArguments;
    StreamList _outputArguments;
    ArgumentList _arguments;

    unsigned int _outWidth;
    unsigned int _outHeight;

    GPURegion _outputRegion;
    std::vector<GPUInterpolant> _inputInterpolants;
  
  };
}

#endif
