// gpukernel.cpp
#include "gpukernel.hpp"

#include "gpu.hpp"

#include <kerneldesc.hpp>
#include <string>

namespace brook
{
  GPUKernel::ArgumentType* GPUKernel::sStreamArgumentType = new GPUKernel::StreamArgumentType();
  GPUKernel::ArgumentType* GPUKernel::sIteratorArgumentType = new GPUKernel::IteratorArgumentType();
  GPUKernel::ArgumentType* GPUKernel::sConstantArgumentType = new GPUKernel::ConstantArgumentType();
  GPUKernel::ArgumentType* GPUKernel::sGatherArgumentType = new GPUKernel::GatherArgumentType();
  GPUKernel::ArgumentType* GPUKernel::sOutputArgumentType = new GPUKernel::OutputArgumentType();

  GPUKernel* GPUKernel::create( GPURuntime* inRuntime, const void* inSource[] )
  {
    BROOK_PROFILE("GPUKernel::create");

    GPUKernel* result = new GPUKernel( inRuntime );
    if( result->initialize( inSource ) )
      return result;
    delete result;
    return NULL;
  }

  GPUKernel::GPUKernel( GPURuntime* inRuntime )
    : _runtime(inRuntime), _context(NULL)
  {
    _context = inRuntime->getContext();
  }

  GPUKernel::~GPUKernel()
  {
    // TODO: implement
  }

  bool GPUKernel::initialize( const void* inSource[] )
  {
//    clearInputs();
//    clearArguments();
    
    size_t i = 0;
    while( inSource[i] != NULL )
    {
      const char* nameString = (const char*)inSource[i];
      const ::brook::desc::gpu_kernel_desc* descriptor = (::brook::desc::gpu_kernel_desc*)inSource[i+1];

      if( descriptor != NULL
        && nameString != NULL
        && _context->isValidShaderNameString( nameString ) )
      {
        return initialize( descriptor );
      }

      i += 2;
    }

    GPUWARN << "Unable to find appropriate GPU kernel descriptor.";
    return false;
  }

  bool GPUKernel::initialize( const ::brook::desc::gpu_kernel_desc* inDescriptor )
  {
    using namespace ::brook::desc;

    _techniques.resize( inDescriptor->_techniques.size() );

    std::vector<gpu_technique_desc>::const_iterator ti = inDescriptor->_techniques.begin();
    std::vector<Technique>::iterator tj = _techniques.begin();
    for(; ti != inDescriptor->_techniques.end(); ++ti, ++tj )
    {
      const gpu_technique_desc& inputTechnique = *ti;
      Technique& outputTechnique = *tj;

      outputTechnique.reductionFactor = inputTechnique._reductionFactor;
      outputTechnique.outputAddressTranslation = inputTechnique._outputAddressTranslation;
      outputTechnique.inputAddressTranslation = inputTechnique._inputAddressTranslation;

      outputTechnique.passes.resize( inputTechnique._passes.size() );
      std::vector<gpu_pass_desc>::const_iterator pi = inputTechnique._passes.begin();
      std::vector<Pass>::iterator pj = outputTechnique.passes.begin();
      for(; pi != inputTechnique._passes.end(); ++pi, ++pj )
      {
        const gpu_pass_desc& inputPass = *pi;
        Pass& outputPass = *pj;

        outputPass.pixelShader = _context->createPixelShader( inputPass._shaderString );
        if( outputPass.pixelShader == NULL )
        {
          GPUWARN << "Failed to create a kernel pass pixel shader";
          return false;
        }

        std::vector<gpu_input_desc>::const_iterator k;
        for( k = inputPass.constants.begin(); k != inputPass.constants.end(); k++ )
          outputPass.addConstant( *k );
        for( k = inputPass.samplers.begin(); k != inputPass.samplers.end(); k++ )
          outputPass.addSampler( *k );
        for( k = inputPass.interpolants.begin(); k != inputPass.interpolants.end(); k++ )
          outputPass.addInterpolant( *k );
        for( k = inputPass.outputs.begin(); k != inputPass.outputs.end(); k++ )
          outputPass.addOutput( *k );
      }
    }

    // initialize the output rects, just in case
//    outputRect = DX9Rect(0,0,0,0);
    return true;
  }

  void GPUKernel::PushStream( Stream* inStream )
  {
    GPUStream* stream = (GPUStream*)inStream;

    size_t streamArgumentIndex = _streamArguments.size();
    _streamArguments.push_back( stream );
    pushArgument( sStreamArgumentType, streamArgumentIndex );
  }

  void GPUKernel::PushIter( Iter* inIterator )
  {
    GPUIterator* iterator = (GPUIterator*)inIterator;

    size_t iteratorArgumentIndex = _iteratorArguments.size();
    _iteratorArguments.push_back( iterator );
    pushArgument( sIteratorArgumentType, iteratorArgumentIndex );
  }

  void GPUKernel::PushConstant( const float& inValue ) {
    PushConstant( float4( inValue, 0, 0, 0 ) );
  }

  void GPUKernel::PushConstant( const float2& inValue ) {
    PushConstant( float4( inValue.x, inValue.y, 0, 0 ) );
  }  
  void GPUKernel::PushConstant( const float3& inValue ) {
    PushConstant( float4( inValue.x, inValue.y, inValue.z, 0 ) );
  }

  void GPUKernel::PushConstant( const float4& inValue )
  {
    size_t constantArgumentIndex = _constantArguments.size();
    _constantArguments.push_back( inValue );
    pushArgument( sConstantArgumentType, constantArgumentIndex );
  }

  void GPUKernel::PushGatherStream( Stream* inStream )
  {
    GPUStream* stream = (GPUStream*)inStream;

    size_t gatherArgumentIndex = _gatherArguments.size();
    _gatherArguments.push_back( stream );
    pushArgument( sGatherArgumentType, gatherArgumentIndex );
  }

  void GPUKernel::PushReduce( void* outValue, __BRTStreamType inType )
  {
    // TODO: handle reductions
    GPUWARN << "Reductions are not implemented\n";
  }

  void GPUKernel::PushOutput( Stream* inStream )
  {
    GPUStream* stream = (GPUStream*)inStream;

    size_t outputArgumentIndex = _outputArguments.size();
    _outputArguments.push_back( stream );
    pushArgument( sOutputArgumentType, outputArgumentIndex );
  }

  void GPUKernel::Map()
  {
    GPUAssert( _outputArguments.size() != 0, "Must have at least some outputs for Map." );

    GPUStream* outputStream = _outputArguments[0];
    _outputRect = outputStream->getOutputRect();

    // Find and execute an appropriate technique
    bool done = false;
    size_t techniqueCount = _techniques.size();
    for( size_t t = 0; t < techniqueCount && !done; t++ )
    {
      Technique& technique = _techniques[t];
      if( technique.inputAddressTranslation == false )
      {
        _context->beginScene();

        executeTechnique( technique );

        _context->endScene();

        done = true;
      }
    }
    if( !done )
      GPUAssert( false, "No appropriate map technique found" );

//    size_t outputStreamCount = _outputStreams.size();
//    for( size_t o = 0; o < outputStreamCount; o++ )
//      outputStreams[o]->markGPUDataChanged();

    clearArguments();
  }

  void GPUKernel::Reduce()
  {
    // TODO: handle reductions
    GPUWARN << "Reductions are not implemented\n";
  }

  /// Internal methods
  void GPUKernel::pushArgument( ArgumentType* inType, size_t inIndex )
  {
    _arguments.push_back( ArgumentInfo( inType, inIndex ) );
  }

  void GPUKernel::clearArguments()
  {
    _streamArguments.clear();
    _iteratorArguments.clear();
    _constantArguments.clear();
    _gatherArguments.clear();
    _outputArguments.clear();
    _arguments.clear();
  }

  void GPUKernel::clearInputs()
  {
    _inputInterpolants.clear();
  }

  void GPUKernel::executeTechnique( const Technique& inTechnique )
  {
    PassList::const_iterator i;
    for( i = inTechnique.passes.begin(); i != inTechnique.passes.end(); i++ )
    {
      executePass( *i );
//      clearInputs();
    }
  }

  void GPUKernel::executePass( const Pass& inPass )
  {
    PixelShaderHandle pixelShader = inPass.pixelShader;
    VertexShaderHandle vertexShader = _context->getPassthroughVertexShader();


    // Bind all the arguments for this pass
    size_t i;

    size_t constantCount = inPass.constants.size();
    for( i = 0; i < constantCount; i++ )
      bindConstant( i, inPass.constants[i] );

    size_t samplerCount = inPass.samplers.size();
    for( i = 0; i < samplerCount; i++ )
      bindSampler( i, inPass.samplers[i] );

    size_t interpolantCount = inPass.interpolants.size();
    _inputInterpolants.resize( interpolantCount );
    for( i = 0; i < interpolantCount; i++ )
      bindInterpolant( i, inPass.interpolants[i] );

    size_t outputCount = inPass.outputs.size();
    for( i = 0; i < outputCount; i++ )
      bindOutput( i, inPass.outputs[i] );
    
    size_t maximumOutputCount = _context->getMaximumOutputCount();
    for( i = outputCount; i < maximumOutputCount; i++ )
      _context->disableOutput( i );

    // Execute
    _context->bindVertexShader( vertexShader );
    _context->bindPixelShader( pixelShader );

    _context->drawRectangle( _outputRect, &_inputInterpolants[0], _inputInterpolants.size() );
  }

  void GPUKernel::bindConstant( size_t inIndex, const Input& inInput )
  {
    if( inInput.argumentIndex > 0 )
    {
      int arg = inInput.argumentIndex-1;
      ArgumentInfo& argument = _arguments[ arg ];
      _context->bindConstant( inIndex, argument.getConstant( this, inInput.componentIndex ) );
    }
  }

  void GPUKernel::bindSampler( size_t inIndex, const Input& inInput )
  {
    if( inInput.argumentIndex > 0 )
    {
      int arg = inInput.argumentIndex-1;
      ArgumentInfo& argument = _arguments[ arg ];
      _context->bindTexture( inIndex, argument.getTexture( this, inInput.componentIndex ) );
    }
  }

  void GPUKernel::bindInterpolant( size_t inIndex, const Input& inInput )
  {
    if( inInput.argumentIndex > 0 )
    {
      int arg = inInput.argumentIndex-1;
      ArgumentInfo& argument = _arguments[ arg ];
      _inputInterpolants[inIndex] = argument.getInterpolant( this, inInput.componentIndex );
    }
  }

  void GPUKernel::bindOutput( size_t inIndex, const Input& inInput )
  {
    if( inInput.argumentIndex > 0 )
    {
      int arg = inInput.argumentIndex-1;
      ArgumentInfo& argument = _arguments[ arg ];
      _context->bindOutput( inIndex, argument.getTexture( this, inInput.componentIndex ) );
    }
  }

  /// Argument Types
  GPUKernel::TextureHandle GPUKernel::ArgumentType::getTexture( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    GPUAssert( false, "No textures in argument" );
    return NULL;
  }

  GPUFatRect GPUKernel::ArgumentType::getInterpolant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    GPUAssert( false, "No interpolant in argument" );
    return GPURect(0,0,0,0);
  }

  float4 GPUKernel::ArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    GPUAssert( false, "No constants in argument" );
    return float4(0,0,0,0);
  }

  // Stream
  GPUKernel::TextureHandle GPUKernel::StreamArgumentType::getTexture( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return inKernel->_streamArguments[ inIndex ]->getIndexedFieldTexture( inComponent );
  }

  GPUFatRect GPUKernel::StreamArgumentType::getInterpolant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    using namespace ::brook::desc;
    GPUStream* stream = inKernel->_streamArguments[ inIndex ];
    switch( inComponent )
    {
    case kStreamInterpolant_Position:
      return stream->getInputInterpolant();
      break;
    }
    return GPURect(0,0,0,0);
  }

  float4 GPUKernel::StreamArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    using namespace ::brook::desc;
    GPUStream* stream = inKernel->_streamArguments[ inIndex ];
    switch( inComponent )
    {
    case kStreamConstant_Indexof:
      return stream->getIndexofConstant();
      break;
    //case kStreamConstant_ATShape:
    //  return stream->getATShapeConstant();
    //  break;
    //case kStreamConstant_ATLinearize:
    //  return stream->getATLinearizeConstant();
    //  break;
    //case kStreamConstant_ATReshape:
    //  return stream->getATReshapeConstant();
    //  break;
    }
    return float4(0,0,0,0);
  }

  // Iterator
  GPUFatRect GPUKernel::IteratorArgumentType::getInterpolant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return GPURect(0,0,0,0);
  }

  float4 GPUKernel::IteratorArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return float4(0,0,0,0);
  }

  // Constant
  float4 GPUKernel::ConstantArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return inKernel->_constantArguments[ inIndex ];
  }

  // Gather
  GPUKernel::TextureHandle GPUKernel::GatherArgumentType::getTexture( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return inKernel->_streamArguments[ inIndex ]->getIndexedFieldTexture( inComponent );
  }

  float4 GPUKernel::GatherArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    using namespace ::brook::desc;
    GPUStream* stream = inKernel->_streamArguments[ inIndex ];
    switch( inComponent )
    {
    case kGatherConstant_Shape:
      return stream->getShapeConstant();
      break;
    }
    return float4(0,0,0,0);
  }

  // Output
  GPUKernel::TextureHandle GPUKernel::OutputArgumentType::getTexture( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return inKernel->_streamArguments[ inIndex ]->getIndexedFieldTexture( inComponent );
  }

  GPUFatRect GPUKernel::OutputArgumentType::getInterpolant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return GPURect(0,0,0,0);
  }

  float4 GPUKernel::OutputArgumentType::getConstant( GPUKernel* inKernel, size_t inIndex, size_t inComponent )
  {
    return float4(0,0,0,0);
  }


}