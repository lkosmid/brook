#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"
#include "dx9texture.hpp"
#include <kerneldesc.hpp>
#include <string>

using namespace brook;

static const char* PIXEL_SHADER_NAME_STRING = "ps20";

DX9Kernel* DX9Kernel::create( DX9RunTime* inRuntime, const void* inSource[] )
{
  DX9PROFILE("DX9Kernel::create")
  DX9Kernel* result = new DX9Kernel( inRuntime );
  if( result->initialize( inSource ) )
    return result;
  delete result;
  return NULL;
}

DX9Kernel::DX9Kernel( DX9RunTime* inRuntime )
  : runtime(inRuntime),
  device(NULL), streamShapeMismatch(false), mustMatchShapeStream(NULL)
{
  device = inRuntime->getDevice();
  device->AddRef();
}

bool DX9Kernel::initialize( const void* inSource[] )
{
  clearInputs();
  clearArguments();
  
  int i = 0;
  while( inSource[i] != NULL )
  {
    const char* nameString = (const char*)inSource[i];
    const ::brook::desc::gpu_kernel_desc* descriptor = (::brook::desc::gpu_kernel_desc*)inSource[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      if( descriptor != NULL )
        return initialize( descriptor );
    }

    i += 2;
  }

  DX9WARN << "Unable to find pixel shader 2.0 code.";
  return false;
}

bool DX9Kernel::initialize( const ::brook::desc::gpu_kernel_desc* inDescriptor )
{
  using namespace ::brook::desc;

  techniques.resize( inDescriptor->_techniques.size() );

  std::vector<gpu_technique_desc>::const_iterator ti = inDescriptor->_techniques.begin();
  std::vector<Technique>::iterator tj = techniques.begin();
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

      outputPass.pixelShader = DX9PixelShader::create( runtime, inputPass._shaderString );
      if( outputPass.pixelShader == NULL )
      {
        DX9WARN << "Failed to create a kernel pass pixel shader";
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

  /* TIM: complete rewrite 


  // must have least one shader
  if( *inProgramStrings == NULL )
  {
    DX9WARN << "No attached ps20 pixel shaders found";
    return false;
  }

  // look for our annotations...
  std::string s = inProgramStrings[0];

  s = s.substr( s.find("!!BRCC") );

  // next is the narg line
  s = s.substr( s.find("\n")+1 );
  s = s.substr( s.find(":")+1 );

  std::string argumentCountString = s.substr( 0, s.find("\n") );
  int argumentCount = atoi( argumentCountString.c_str() );

  int i;
  for( i = 0; i < argumentCount; i++ )
  {
    s = s.substr( s.find("\n")+1 );
    s = s.substr( s.find("//")+2 );
    char typeCode = s[0];
    char indexofHint = s[1];
    argumentUsesIndexof.push_back( (indexofHint == 'i') );
  }

  const char** programStrings = inProgramStrings;
  while( *programStrings != NULL )
  {
    const char* programString = *programStrings++;

    Pass pass;
    pass.pixelShader = DX9PixelShader::create( runtime, programString );
    if( pass.pixelShader == NULL )
    {
      DX9WARN << "Failed to create a kernel pass pixel shader";
      return false;
    }

    std::string s = programString;
    s = s.substr( s.find("!!multipleOutputInfo") );
    s = s.substr( s.find(":")+1 );
    pass.firstOutput = atoi( s.substr( 0, s.find(":") ).c_str() );
    s = s.substr( s.find(":")+1 );
    pass.outputCount = atoi( s.substr( 0, s.find(":") ).c_str() );

    s = s.substr( s.find("!!fullAddressTrans") );
    s = s.substr( s.find(":")+1 );
    int addressTrans = atoi( s.substr( 0, s.find(":") ).c_str() );

    s = s.substr( s.find("!!reductionFactor") );
    s = s.substr( s.find(":")+1 );
    int reductionFactor = atoi( s.substr( 0, s.find(":") ).c_str() );

    if( reductionFactor )
    {
      addReductionPass( reductionFactor, addressTrans != 0, pass );
    }
    else if( !addressTrans )
    {
      standardPasses.push_back(pass);
    }
    else
    {
      fullTranslationPasses.push_back(pass);
    }
  }
*/
  // initialize the output rects, just in case
  outputRect = DX9Rect(0,0,0,0);
  return true;
}
/*
void DX9Kernel::addReductionPass( int inReductionFactor, bool inFullTranslation, const Pass& inPass )
{
  int reductionPassCount = reductionPasses.size();
  for( int i = 0; i < reductionPassCount; i++ )
  {
    if( reductionPasses[i].reductionFactor != inReductionFactor ) continue;

    if( inFullTranslation )
      reductionPasses[i].fullTranslationPasses.push_back( inPass );
    else
      reductionPasses[i].standardPasses.push_back( inPass );
    return;
  }

  // no such reduction pass
  ReductionPass reductionPass;
  reductionPass.reductionFactor = inReductionFactor;
  if( inFullTranslation )
    reductionPass.fullTranslationPasses.push_back( inPass );
  else
    reductionPass.standardPasses.push_back( inPass );
  reductionPasses.push_back( reductionPass );
}*/

DX9Kernel::~DX9Kernel()
{
  // TIM: TODO: release it all...
/*  int p,passCount = (int)standardPasses.size();
  for( p = 0; p < passCount; p++ )
    delete standardPasses[p].pixelShader;
  passCount = (int)fullTranslationPasses.size();
  for( p = 0; p < passCount; p++ )
    delete fullTranslationPasses[p].pixelShader;
*/
  if( device != NULL )
    device->Release();
}

void DX9Kernel::PushStream(Stream *s)
{
  DX9PROFILE("DX9Kernel::PushStream")
  DX9Stream* stream = (DX9Stream*)s;

  // argument
  pushStreamArgument( stream );

  // reduction data
  inputReductionArgumentIndex = getCurrentArgumentIndex();

  // constants
  pushConstantImpl( stream->getIndexofConstant() ); // kStreamConstant_Indexof
  pushATShapeConstant( stream ); // kStreamConstant_ATShape
  pushConstantImpl( stream->getATLinearizeConstant() ); // kStreamConstant_ATLinearize
  pushConstantImpl( stream->getATReshapeConstant() ); // kStreamConstant_ATReshape
  
  // samplers
  pushSamplers( stream );

  // interpolants
  pushInterpolant( stream->getInputRect() );

  // no outputs
/*
  inputStreams.push_back(stream);

  // reduction stream is always the first/last/only stream pushed
  inputReductionStream = stream;
  inputReductionStreamSamplerIndex = (int)inputTextures.size();
  inputReductionStreamTexCoordIndex = (int)inputTextureRects.size();

  if( runtime->isAddressTranslationOn() )
  {
    PushSamplers( stream );
    int shapeConstantIndex = PushConstantImpl( float4(0,0,0,0) );
    inputStreamShapeConstantIndices.push_back( shapeConstantIndex );
    PushConstantImpl( stream->getATLinearizeConstant() );
    PushConstantImpl( stream->getATReshapeConstant() );

    matchStreamShape( stream );
  }
  else
  {
    PushSamplers( stream );
    PushTexCoord( stream->getInputRect() );
    if( argumentUsesIndexof[arg] )
      PushConstantImpl( stream->getIndexofConstant() );
  }
*/
}

void DX9Kernel::PushIter(class Iter * v)
{
  DX9PROFILE("DX9Kernel::PushIter")
  DX9Iter* iterator = (DX9Iter*)v;

  // argument
  pushArgument();

  // no constants
  // no samplers
  // interpolants
  pushInterpolant( iterator->getRect() );
  // no outputs
/*
  PushTexCoord( iterator->getRect() );
  */
}

void DX9Kernel::PushReduce(void * val, __BRTStreamType type) {
  DX9PROFILE("DX9Kernel::PushReduce")
  DX9LOG(1) << "PushReduce";

  // argument
  pushArgument();

  // reduction data
  argumentReductions.push_back( ReductionArgumentInfo( val, type ) );

  outputRedctionArgumentIndex = getCurrentArgumentIndex();

  /* TIM: TODO: reimplement
  outputReductionDatas.push_back(val);
  outputReductionTypes.push_back(type);

  outputReductionArgumentIndex = argumentIndex;
  */

  // no constants
  // no samplers
  // no interpolants
  // no outputs
}

void DX9Kernel::PushConstant(const float &val) {
  float4 value;
  value.x = val;
  value.y = 0;
  value.z = 0;
  value.w = 1;
  PushConstant( value );
}

void DX9Kernel::PushConstant(const float2 &val) {
  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = 0;
  value.w = 1;
  PushConstant( value );
}

void DX9Kernel::PushConstant(const float3 &val) {
  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = val.z;
  value.w = 1;
  PushConstant( value );
}

void DX9Kernel::PushConstant(const float4 &val) {
  DX9PROFILE("DX9Kernel::PushConstant")

  // argument
  pushArgument();

  // constants
  pushConstantImpl(val);
  // no samplers
  // no interpolants
  // no outputs
}

void DX9Kernel::PushGatherStream(Stream *s) {
  DX9PROFILE("DX9Kernel::PushGatherStream")
  DX9Stream* stream = (DX9Stream*)s;

  // argument
  pushStreamArgument( stream );

  // constants
  pushConstantImpl( stream->getGatherConstant() ); // kGatherConstant_Gather
  pushConstantImpl( stream->getATLinearizeConstant() ); // kGatherConstant_ATLinearize
  pushConstantImpl( stream->getATReshapeConstant() ); // kGatherCosntant_ATReshape
  // samplers
  pushSamplers( stream );
  // no interpolants
  // no outputs

/* TIM: rewrite 
  inputStreams.push_back(stream);
  if( runtime->isAddressTranslationOn() )
  {
    PushSamplers( stream );
    inputStreamShapeConstantIndices.push_back( -1 );
    PushConstantImpl( stream->getATLinearizeConstant() );
    PushConstantImpl( stream->getATReshapeConstant() );
  }
  else
  {
    PushConstantImpl( stream->getGatherConstant() );
    PushSamplers( stream );
  }
*/
}

void DX9Kernel::PushOutput(Stream *s) {
  DX9PROFILE("DX9Kernel::PushOutput")
  DX9LOG(1) << "PushOutput";

  DX9Stream* stream = (DX9Stream*)s;

  pushStreamArgument( stream );
/* pushOutput should handle this
  if( argumentOutputs.size() == 0 )
  {
    // first one
    outputRect = stream->getOutputRect();
  }
  else
  {
    DX9Stream* first = outputStreams[0];
    DX9Assert( first->getDimension() == stream->getDimension(),
      "Output streams do not have matching dimensionality" );
    int d = first->getDimension();
    for( int i = 0; i < d; i++ )
    {
      DX9Assert( first->getExtents()[i] == stream->getExtents()[i],
        "Output streams do not have matching extents" );
    }
  }
*/
  // constants
  pushConstantImpl( stream->getIndexofConstant() ); // kOutputConstant_Indexof
  // no samplers
  // no interpolants
  pushInterpolant( stream->getInputRect() ); // kOutputInterpolant_Position
  // outputs
  pushOutputs( stream );

  // TIM: TODO: this constant and interpolant are actually "global"
  // since they are shared between all outputs...
//  PushConstantImpl( stream->getIndexofConstant() );
//  PushTexCoord( stream->getInputRect() );

/*
  outputStreams.push_back(stream);

  int surfaceCount = stream->getSubstreamCount();
  for( int i = 0; i < surfaceCount; i++ )
  {
    IDirect3DSurface9* surface = stream->getIndexedSurfaceHandle( i );

    outputSurfaces.push_back(surface);
  }

  if( runtime->isAddressTranslationOn() )
  {
    matchStreamShape( stream );
  }

  if( !runtime->isAddressTranslationOn() && argumentUsesIndexof[arg] && !hasPushedOutputIndexof )
  {
    hasPushedOutputIndexof = true;
    PushConstantImpl( stream->getIndexofConstant() );
    PushTexCoord( stream->getInputRect() );
  }
  */
}

void DX9Kernel::mapTechnique( const DX9Kernel::Technique& inTechnique )
{
  DX9PROFILE("DX9Kernel::mapTechnique");

  std::vector<Pass>::const_iterator i;
  for( i = inTechnique.passes.begin(); i != inTechnique.passes.end(); i++ )
  {
    mapPass( *i );
    clearInputs();
  }
}

void DX9Kernel::mapPass( const DX9Kernel::Pass& inPass )
{
  DX9PROFILE("DX9Kernel::mapPass")
  HRESULT result;
  size_t i;

  DX9PixelShader* pixelShader = inPass.pixelShader;
  DX9VertexShader* vertexShader = runtime->getPassthroughVertexShader();

  // Bind all the arguments for this pass
  size_t constantCount = inPass.constants.size();
  for( i = 0; i < constantCount; i++ )
    bindConstant( i, inPass.constants[i] );

  size_t samplerCount = inPass.samplers.size();
  for( i = 0; i < samplerCount; i++ )
    bindSampler( i, inPass.samplers[i] );

  size_t interpolantCount = inPass.interpolants.size();
  for( i = 0; i < interpolantCount; i++ )
    bindInterpolant( i, inPass.interpolants[i] );

  size_t outputCount = inPass.outputs.size();
  for( i = 0; i < outputCount; i++ )
    bindOutput( i, inPass.outputs[i] );
  static const size_t kMaximumOutputCount = 4;
  for( i = outputCount; i < kMaximumOutputCount; i++ )
    disableOutput( i );

  // Execute

  result = device->SetPixelShader( pixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );
  result = device->SetVertexShader( vertexShader->getHandle() );
  DX9AssertResult( result, "SetVertexShader failed" );

  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

  /* TIM: complete rewrite...

  int i;

  int inputStreamCount = (int)inputStreams.size();
  int samplerCount = (int)inputTextures.size();
  int texCoordCount = (int)inputTextureRects.size();
  int constantCount = (int)inputConstants.size();
  int reductionCount = (int)outputReductionDatas.size();

  DX9PixelShader* pixelShader = inPass.pixelShader;
  DX9VertexShader* vertexShader = runtime->getPassthroughVertexShader();

  int firstOutput = inPass.firstOutput;
  int outputCount = inPass.outputCount;
  int afterLastOutput = inPass.firstOutput + outputCount;

  for( i = firstOutput; i < afterLastOutput; i++ )
  {
    DX9Assert( i < (int)outputSurfaces.size(), "Not enough output surfaces have been pushed!" );
    result = device->SetRenderTarget( i-firstOutput, outputSurfaces[i] );
    DX9AssertResult( result, "SetRenderTarget failed" );
  }
  static const int kMaximumRenderTargetCount = 4;
  for( i = outputCount; i < kMaximumRenderTargetCount; i++ )
  {
    result = device->SetRenderTarget( i, NULL );
    DX9AssertResult( result, "SetRenderTarget(NULL) failed" );
  }


  result = device->BeginScene();
  DX9AssertResult( result, "BeginScene failed"  );

  result = device->SetPixelShader( pixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );
  result = device->SetVertexShader( vertexShader->getHandle() );
  DX9AssertResult( result, "SetVertexShader failed" );
  for( i = 0; i < inputStreamCount; i++ )
    inputStreams[i]->validateGPUData();
  for( i = 0; i < samplerCount; i++ )
  {
    result = device->SetTexture( i, inputTextures[i] );
    DX9AssertResult( result, "SetTexture failed" );
  }

  for( i = 0; i < constantCount; i++ )
  {
    result = device->SetPixelShaderConstantF( i, (float*)&(inputConstants[i]), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }

  // TIM: The workspace constant is dead, long live the workspace constant...
  // we now put any special or "magical" constants (like the deceased WS
  // constant or the blasphemous adress-translation cosntants) *after*
  // the constants consumed by normal arguments.
  if( runtime->isAddressTranslationOn() )
  {
    DX9Stream* outputStream = outputStreams[0];

    float4 outputConstant = outputStream->getATOutputConstant();
    float4 hackConstant(1,1,1,1);

    result = device->SetPixelShaderConstantF( constantCount+0, (float*)&(outputConstant), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
    result = device->SetPixelShaderConstantF( constantCount+1, (float*)&(hackConstant), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }


  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

  result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );
*/
}

void DX9Kernel::bindConstant( size_t inConstantIndex, const Input& inInput )
{
  float4 value = getConstant( inInput.argumentIndex, inInput.componentIndex );

  DX9LOG(0) << "Binding constant " << inConstantIndex << " to value {"
    << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "}";

  HRESULT result = device->SetPixelShaderConstantF( inConstantIndex, (float*)&(value), 1 );
  DX9AssertResult( result, "SetPixelShaderConstantF failed" );
}

void DX9Kernel::bindSampler( size_t inSamplerIndex, const Input& inInput )
{
  IDirect3DBaseTexture9* texture = getSampler( inInput.argumentIndex, inInput.componentIndex );
  HRESULT result = device->SetTexture( inSamplerIndex, texture );
  DX9AssertResult( result, "SetTexture failed" );
}

void DX9Kernel::bindInterpolant( size_t inInterpolantIndex, const Input& inInput )
{
  DX9FatRect rectangle = getInterpolant( inInput.argumentIndex, inInput.componentIndex );

  DX9LOG(0) << "Binding interpolant " << inInterpolantIndex << " to rectangle: " << std::endl
    << "v0 = {" << rectangle.vertices[0].x << ", " << rectangle.vertices[0].y << "}" << std::endl
    << "v1 = {" << rectangle.vertices[1].x << ", " << rectangle.vertices[1].y << "}" << std::endl
    << "v1 = {" << rectangle.vertices[2].x << ", " << rectangle.vertices[2].y << "}" << std::endl
    << "v2 = {" << rectangle.vertices[3].x << ", " << rectangle.vertices[3].y << "}" << std::endl;

  inputTextureRects.push_back( rectangle );
}

void DX9Kernel::bindOutput( size_t inOutputIndex, const Input& inInput )
{
  IDirect3DSurface9* surface = getOutput( inInput.argumentIndex, inInput.componentIndex );
  HRESULT result = device->SetRenderTarget( inOutputIndex, surface );
  DX9AssertResult( result, "SetRenderTarget failed" );
}

void DX9Kernel::disableOutput( size_t inOutputIndex )
{
  HRESULT result = device->SetRenderTarget( inOutputIndex, NULL );
  DX9AssertResult( result, "SetRenderTarget(NULL) failed" );
}

float4 DX9Kernel::getConstant( int inArgument, int inComponent )
{
  if( inArgument > 0 )
  {
    int arg = inArgument-1;
    DX9Assert( inComponent < arguments[arg].constantCount, "not enough constants in argument" );
    return argumentConstants[ arguments[arg].firstConstantIndex + inComponent ];
  }
/*  else if( inArgument == 0 )
  {
    return getGlobalConstant( inComponent );
  }
  else
  {
  }*/
  return float4(0,0,0,0);
}

IDirect3DBaseTexture9* DX9Kernel::getSampler( int inArgument, int inComponent )
{
  if( inArgument > 0 )
  {
    int arg = inArgument-1;
    DX9Assert( inComponent < arguments[arg].samplerCount, "not enough samplers in argument" );
    DX9Stream* stream = argumentStreams[arg];
    if( stream ) stream->validateGPUData();
    return argumentSamplers[ arguments[arg].firstSamplerIndex + inComponent ];
  }
  else if( inArgument == 0 )
  {
    DX9Assert( inComponent < (int)globalSamplers.size(), "not enough global samplers" );
    return globalSamplers[inComponent];
  }
  return 0;
}

DX9FatRect DX9Kernel::getInterpolant( int inArgument, int inComponent )
{
  if( inArgument > 0 )
  {
    int arg = inArgument-1;
    DX9Assert( inComponent < arguments[arg].interpolantCount, "not enough interpolants in argument" );
    return argumentInterpolants[ arguments[arg].firstInterpolantIndex + inComponent ];
  }
  else if( inArgument == 0 )
  {
    DX9Assert( inComponent < (int)globalInterpolants.size(), "not enough global interpolants" );
    return globalInterpolants[inComponent];
  }

  return DX9Rect(0,0,0,0);
}

IDirect3DSurface9* DX9Kernel::getOutput( int inArgument, int inComponent )
{
  if( inArgument > 0 )
  {
    int arg = inArgument-1;
    DX9Assert( inComponent < arguments[arg].outputCount, "not enough outputs in argument" );
    return argumentOutputs[ arguments[arg].firstOutputIndex + inComponent ];
  }
  else if( inArgument == 0 )
  {
    DX9Assert( inComponent < (int)globalOutputs.size(), "not enoguh global outputs" );
    return globalOutputs[inComponent];
  }
  return 0;
}

void DX9Kernel::Map() {
  DX9PROFILE("DX9Kernel::Map")
  DX9LOG(1) << "Map";

  assert(outputStreams.size() > 0);
  DX9Stream* outputStream = outputStreams[0];
  outputRect = outputStream->getOutputRect();

  // push global data...
  
  // constants
  globalConstants.push_back( float4(0,0,0,0) ); // kGlobalConstants_Workspace
  globalConstants.push_back( outputStream->getIndexofConstant() ); // kGlobalConstant_OutputIndexof;
  // no samplers
  // interpolants
  globalInterpolants.push_back( outputStream->getInputRect() ); // kGlobalInterpolant_OutputTextureCoordinate
  globalInterpolants.push_back( outputStream->getATAddressInterpolantRect() ); // kGlobalInterpolant_ATOutputAddress;
  // no outputs

  // There are certain constants that cannot be specified until we know the shape of the output
  // we now go back through and fill them in...
  size_t atShapeConstantCount = atShapeConstants.size();
  for( size_t i = 0; i < atShapeConstantCount; i++ )
  {
    DX9Stream* stream = argumentStreams[ atShapeConstants[i] ];
    size_t constantIndex = arguments[ atShapeConstants[i] ].firstConstantIndex + ::brook::desc::kStreamConstant_ATShape;
    argumentConstants[constantIndex] = stream->getATShapeConstant( outputStream->getATOutputShape() );
  }

  // Find and execute an appropriate technique
  bool done = false;
  size_t techniqueCount = techniques.size();
  for( size_t t = 0; t < techniqueCount && !done; t++ )
  {
    Technique& technique = techniques[t];
    if( technique.inputAddressTranslation == streamShapeMismatch )
    {
      HRESULT result = device->BeginScene();
      DX9AssertResult( result, "BeginScene failed" );

      mapTechnique( technique );

      result = device->EndScene();
      DX9AssertResult( result, "EndScene failed" );

      done = true;
    }
  }
  if( !done )
    DX9Assert( false, "No appropriate map technique found" );

  size_t outputStreamCount = outputStreams.size();
  for( size_t o = 0; o < outputStreamCount; o++ )
    outputStreams[o]->markGPUDataChanged();

  clearArguments();

  /* TIM: rewrite

  if( runtime->isAddressTranslationOn() )
  {
    DX9Stream* outputStream = outputStreams[0];

    inputTextureRects.push_back( outputStream->getInputRect() ); // standard texcoord
    inputTextureRects.push_back( outputStream->getATAddressInterpolantRect() ); // special magic

    float4 outputShape = outputStream->getATOutputShape();
    int streamCount = (int)inputStreams.size();
    for( int s = 0; s < streamCount; s++ )
    {
      DX9Stream* stream = inputStreams[s];
      int index = inputStreamShapeConstantIndices[s];
      if( index == -1 ) continue; // gather stream
      float4 shapeConstant = stream->getATShapeConstant( outputShape );
      inputConstants[(size_t)index] = shapeConstant;
    }
  }

  if( streamShapeMismatch )
  {
    int passCount = (int)fullTranslationPasses.size();
    if( passCount == 0 )
    {
      DX9Assert(false,
        "A kernel was called with unaligned streams for which a replication-stride "
        "set of shader passes was not generated.");
    }
    for( int p = 0; p < passCount; p++ )
      mapPass( fullTranslationPasses[p] );
  }
  else
  {
    int passCount = (int)standardPasses.size();
    if( passCount == 0 )
    {
      DX9Assert(false,"No untranslated passes found to execute");
    }
    for( int p = 0; p < passCount; p++ )
      mapPass( standardPasses[p] );
  }

  int outputStreamCount = (int)outputStreams.size();
  for( int i = 0; i < outputStreamCount; i++ )
  {
    outputStreams[i]->markGPUDataChanged();
  }

  ClearInputs();
  */
}

void DX9Kernel::Reduce() {
  DX9PROFILE("DX9Kernel::Reduce")
  DX9LOG(1) << "Reduce";

  DX9Assert( argumentReductions.size() == 1,
    "Must have one and only one reduction output." );

  ReductionArgumentInfo reductionArgument = argumentReductions[0];
  __BRTStreamType outputReductionType = reductionArgument.type;
  void* outputReductionData = reductionArgument.data;

  if( outputReductionType == __BRTSTREAM )
  {
    Stream* outputStreamBase = *((const ::brook::stream*)outputReductionData);
    DX9Stream* outputStream = (DX9Stream*)outputStreamBase;
    DX9Assert(outputStream->getSubstreamCount() == 1,
      "Cannot reduce to a structure right now");
    DX9Texture* outputTexture = outputStream->getIndexedTexture(0);
    ReduceToStream( outputTexture );
  }
  else
  {
    DX9Texture* outputTexture = runtime->getReductionTargetBuffer();
    ReduceToStream( outputTexture );

    float4 reductionResult;
    outputTexture->getData( (float*)&reductionResult, sizeof(float4), 1 );
    if( outputReductionType == __BRTFLOAT )
      *((float*)outputReductionData) = *((float*)&reductionResult);
    else if( outputReductionType == __BRTFLOAT2 )
      *((float2*)outputReductionData) = *((float2*)&reductionResult);
    else if( outputReductionType == __BRTFLOAT3 )
      *((float3*)outputReductionData) = *((float3*)&reductionResult);
    else if( outputReductionType == __BRTFLOAT4 )
      *((float4*)outputReductionData) = *((float4*)&reductionResult);
    else
    {
      DX9Assert(false,"Invalid reduction target type.\n"
        "Only float, float2, float3, and float4 outputs allowed.");
    }
  }
}

DX9Kernel::ArgumentInfo& DX9Kernel::getCurrentArgument() {
  return *(arguments.rbegin());
}

size_t DX9Kernel::getCurrentArgumentIndex() {
  return arguments.size()-1;
}

void DX9Kernel::pushArgument()
{
  pushStreamArgument( 0 );
}

void DX9Kernel::pushStreamArgument( DX9Stream* inStream )
{
  ArgumentInfo argumentInfo;
  argumentInfo.firstConstantIndex = argumentConstants.size();
  argumentInfo.firstSamplerIndex = argumentSamplers.size();
  argumentInfo.firstInterpolantIndex = argumentInterpolants.size();
  argumentInfo.firstOutputIndex = argumentOutputs.size();

  argumentInfo.constantCount = 0;
  argumentInfo.samplerCount = 0;
  argumentInfo.interpolantCount = 0;
  argumentInfo.outputCount = 0;

  argumentStreams.push_back( inStream );
  arguments.push_back( argumentInfo );
}

void DX9Kernel::pushSamplers( DX9Stream* s )
{
  DX9PROFILE("DX9Kernel::pushSamplers")

  int samplerCount = s->getSubstreamCount();
  getCurrentArgument().samplerCount += samplerCount;
  for( int i = 0; i < samplerCount; i++ )
    argumentSamplers.push_back( s->getIndexedTextureHandle(i) );
}

void DX9Kernel::pushInterpolant( const DX9FatRect& r )
{
  DX9PROFILE("DX9Kernel::PushTexCoord")
  
  getCurrentArgument().interpolantCount++;
  argumentInterpolants.push_back( r );
}

void DX9Kernel::pushConstantImpl(const float4 &val)
{
  getCurrentArgument().constantCount++;
  argumentConstants.push_back( val );
}

void DX9Kernel::pushATShapeConstant( DX9Stream* s )
{
  // push the index of the current argument...
  atShapeConstants.push_back( arguments.size() - 1 );
  pushConstantImpl( float4(0,0,0,0) );
}

void DX9Kernel::pushOutputs( DX9Stream* s )
{
  DX9PROFILE("DX9Kernel::pushSamplers")

  int surfaceCount = s->getSubstreamCount();
  getCurrentArgument().outputCount += surfaceCount;
  for( int i = 0; i < surfaceCount; i++ )
    argumentOutputs.push_back( s->getIndexedSurfaceHandle(i) );
  outputStreams.push_back( s );
}

void DX9Kernel::clearInputs()
{
  DX9PROFILE("DX9Kernel::clearInputs");

  inputTextureRects.clear();
}

void DX9Kernel::clearArguments()
{
  DX9PROFILE("DX9Kernel::ClearInputs")

  arguments.clear();
  argumentConstants.clear();
  argumentSamplers.clear();
  argumentInterpolants.clear();
  argumentOutputs.clear();
  argumentStreams.clear();
  atShapeConstants.clear();
  outputStreams.clear();
  argumentReductions.clear();

/*
  argumentIndex = 0;
  hasPushedOutputIndexof = false;
  mustMatchShapeStream = NULL;
  streamShapeMismatch = false;

  outputStreams.clear();
  outputSurfaces.clear();
  inputTextureRects.clear();
  inputConstants.clear();
  inputStreams.clear();
  inputStreamShapeConstantIndices.clear();
  inputTextures.clear();
  outputReductionDatas.clear();
  outputReductionTypes.clear();

  */
}

void DX9Kernel::matchStreamShape( DX9Stream* inStream )
{
  if( mustMatchShapeStream == NULL )
  {
    mustMatchShapeStream = inStream;
    return;
  }
  else
  {
    int dimensionCount = inStream->getDimension();
    if( dimensionCount != mustMatchShapeStream->getDimension() )
    {
      DX9Assert( false, "Input streams must match in dimensionality" );
    }
    for( int d = 0; d < dimensionCount; d++ )
    {
      if( inStream->getExtents()[d] != mustMatchShapeStream->getExtents()[d] )
      {
       streamShapeMismatch = true;
        return;
      }
    }
  }
}

void DX9Kernel::ReduceToStream( DX9Texture* inOutputBuffer )
{
  DX9PROFILE("DX9Kernel::ReduceToStream")
  DX9Texture* outputBuffer = inOutputBuffer;
  int outputWidth = outputBuffer->getWidth();
  int outputHeight = outputBuffer->getHeight();

  DX9Stream* inputStream = argumentStreams[inputReductionArgumentIndex];
  // TIM: we can't assume knowledge of what resources these bind to
  // (at least not officially) :)
/*  int tex0 = inputReductionStreamTexCoordIndex;
  int tex1 = outputReductionVarTexCoordIndex;
  if( tex0 > tex1 )
  {
    int temp = tex0;
    tex0 = tex1;
    tex1 = temp;
  }
  int sampler0 = inputReductionStreamSamplerIndex;
  int sampler1 = outputReductionVarSamplerIndex;
  if( sampler0 > sampler1 )
  {
    int temp = sampler0;
    sampler0 = sampler1;
    sampler1 = temp;
  }*/
  int inputWidth = inputStream->getTextureWidth();
  int inputHeight = inputStream->getTextureHeight();

  int xFactor = inputWidth / outputWidth;
  int yFactor = inputHeight / outputHeight;
  
  int firstDimension = 0;
  int secondDimension = 1;
  if( yFactor > xFactor )
  {
    firstDimension = 1;
    secondDimension = 0;
  }

  ReductionState state;
  state.inputTexture = inputStream->getIndexedTexture(0);
  state.outputTexture = outputBuffer;
  state.whichBuffer = -1; // data starts in the input
  state.reductionBuffers[0] = NULL;
  state.reductionBuffers[1] = NULL;
  state.slopBuffer = NULL;
//  state.leftTextureUnit = tex0;
//  state.rightTextureUnit = tex1;
//  state.leftSampler = sampler0;
//  state.rightSampler = sampler1;
  state.currentDimension = firstDimension;
  state.targetExtents[0] = outputWidth;
  state.targetExtents[1] = outputHeight;
  state.inputExtents[0] = inputWidth;
  state.inputExtents[1] = inputHeight;
  state.currentExtents[0] = inputWidth;
  state.currentExtents[1] = inputHeight;
  state.slopCount = 0;

  beginReduction( state );

  while( state.currentExtents[firstDimension] != state.targetExtents[firstDimension] )
    executeReductionStep( state );
  executeSlopStep( state );
  state.currentDimension = secondDimension;
  while( state.currentExtents[secondDimension] != state.targetExtents[secondDimension] )
    executeReductionStep( state );
  executeSlopStep( state );

  endReduction( state );
}

void DX9Kernel::executeReductionTechnique( int inFactor )
{
  DX9PROFILE("DX9Kernel::executeReductionTechnique");

  // we assume for now that the techniques are stored
  // starting sequentiall with the 2-way reduction
  Technique& technique = techniques[ inFactor - 2 ];
  mapTechnique( technique );
}

void DX9Kernel::beginReduction( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::beginReduction")

  HRESULT result = device->BeginScene();
  DX9AssertResult( result, "BeginScene failed" );

  ioState.inputTexture->validateCachedData();

  // set up
  // TIM: most of this work gets pushed into the per-pass stuff now...
  // for better or worse... :(
/*  

  int i,constantCount = (int)inputConstants.size();
  int outputCount = (int)outputStreams.size();
  int reductionCount = (int)outputReductionTypes.size();
  int samplerCount = (int)inputTextures.size();

  // inspect the input stuff:
  DX9Assert( reductionCount == 1, "Number of 'reduce' arguments was not 1." );
  DX9Assert( outputCount == 0, "'out' streams found in reduce function." );

  DX9VertexShader* passthroughVertexShader = runtime->getPassthroughVertexShader();
  result = device->SetVertexShader( passthroughVertexShader->getHandle() );
  DX9AssertResult( result, "SetVertexShader failed" );

  for( i = 0; i < constantCount; i++ )
  {
    result = device->SetPixelShaderConstantF( i, (float*)&(inputConstants[i]), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }

  for( i = 0; i < samplerCount; i++ )
  {
    if( i == ioState.leftSampler || i == ioState.rightSampler ) continue;
    inputStreams[i]->validateGPUData();
    result = device->SetTexture( i, inputTextures[i] );
    DX9AssertResult( result, "SetTexture failed" );
  }

#ifdef BROOK_DX9_TRACE_REDUCE
  dumpReductionState( ioState );
#endif
*/
}

void DX9Kernel::executeReductionStep( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::executeReductionStep")
  DX9LOG(3) << "Reduction Step";

  int dim = ioState.currentDimension;
  int remainingFactor = ioState.currentExtents[dim] / ioState.targetExtents[dim];
  int remainingExtent = ioState.currentExtents[dim];
  int outputExtent = ioState.targetExtents[dim];
  int otherExtent = ioState.currentExtents[1-dim];
//  int tex0 = ioState.leftTextureUnit;
//  int tex1 = ioState.rightTextureUnit;

  // Find the appropriate technique to execute:
  // we assume here that the techniques are ordered
  // from worst to best...
  std::vector<Technique>::reverse_iterator t;
  for( t = techniques.rbegin(); t != techniques.rend(); ++t )
  {
    Technique& technique = *t;
    int passFactor = technique.reductionFactor;

    int quotient = remainingFactor / passFactor;
    int remainder = remainingFactor % passFactor;

    if( quotient == 0 ) continue;
    if( quotient == 1 || remainder <= 1 ) break;
  }

  if( t == techniques.rend() )
    DX9Assert( false, "There was no available reduction pass... this should never happen");

  Technique& technique = *t;
  int reductionFactor = technique.reductionFactor;
  int slopFactor = (remainingFactor % reductionFactor);

  int resultExtents[2];
  resultExtents[0] = ioState.currentExtents[0];
  resultExtents[1] = ioState.currentExtents[1];
  resultExtents[dim] = outputExtent * (remainingFactor / reductionFactor);

  // we need to set up the resources so that this technique can execute...
  // we will put them all in "global" storage so that we can access
  // them easily from the technique

  // find the proper textures for the shader inputs
  DX9Texture* slopBuffer = ioState.slopBuffer;
  DX9Texture* inputBuffer = NULL;
  if( ioState.whichBuffer == -1 )
    inputBuffer = ioState.inputTexture;
  else
    inputBuffer = ioState.reductionBuffers[ioState.whichBuffer];

  int nextBuffer = (ioState.whichBuffer + 1) % 2;
  DX9Texture* outputBuffer = ioState.reductionBuffers[nextBuffer];
  if( outputBuffer == NULL )
  {
    if( nextBuffer == 0 )
      outputBuffer = runtime->getReductionBuffer0( resultExtents[0], resultExtents[1] );
    else
      outputBuffer = runtime->getReductionBuffer1( resultExtents[0], resultExtents[1] );
    ioState.reductionBuffers[nextBuffer] = outputBuffer;
  }

  globalSamplers.resize(3);
  globalSamplers[0] = inputBuffer->getTextureHandle();
  globalSamplers[1] = inputBuffer->getTextureHandle();
  
  globalOutputs.resize(1);
  globalOutputs[0] = outputBuffer->getSurfaceHandle();

  globalInterpolants.resize( reductionFactor );
  for( int i = 0; i < reductionFactor; i++ )
  {
    globalInterpolants[i] = inputBuffer->getReductionTextureRect(
      i, remainingExtent+i, 0, otherExtent, dim );
  }
  int newExtent = resultExtents[dim];
  ioState.currentExtents[dim] = newExtent;

  outputRect = outputBuffer->getReductionSurfaceRect(
    0, newExtent, 0, otherExtent, dim );

  // use the existing map functionality to execute the passes... :)
  mapTechnique( technique );

  // move any slop out to the slop buffer
  if( slopFactor )
  {
    if( slopBuffer == NULL )
    {
      int slopExtents[2];
      slopExtents[dim] = outputExtent;
      slopExtents[1-dim] = otherExtent;

      slopBuffer = runtime->getSlopBuffer( slopExtents[0], slopExtents[1] );
      ioState.slopBuffer = slopBuffer;
    }

    if( ioState.slopCount == 0 && slopFactor ==1 )
    {
      // first time, easy case
      device->SetPixelShader( runtime->getPassthroughPixelShader()->getHandle() );
      device->SetTexture( 0, inputBuffer->getTextureHandle() );

      int offset = remainingFactor-1;
      inputTextureRects.push_back( inputBuffer->getReductionTextureRect( offset, remainingExtent+offset, 0, otherExtent, dim ) );

      device->SetRenderTarget( 0, slopBuffer->getSurfaceHandle() );
      outputRect = slopBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );

      runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
      clearInputs();
      ioState.slopCount++;
    }
    else if( ioState.slopCount == 0 )
    {
      // first time, normal case...

      globalSamplers[0] = inputBuffer->getTextureHandle();
      globalSamplers[1] = inputBuffer->getTextureHandle();
      globalOutputs[0] = slopBuffer->getSurfaceHandle();

      for( int i = 0; i < slopFactor; i++ )
      {
        int offset = slopFactor - i;
        offset = remainingFactor - offset;
        globalInterpolants[i] = inputBuffer->getReductionTextureRect(
          offset, remainingExtent + offset, 0, otherExtent, dim );
      }
      outputRect = slopBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );

      executeReductionTechnique( slopFactor );
      ioState.slopCount++;
    }
    else
    {
      globalSamplers[0] = inputBuffer->getTextureHandle();
      globalSamplers[1] = slopBuffer->getTextureHandle();
      globalOutputs[0] = slopBuffer->getSurfaceHandle();

      for( int i = 0; i < slopFactor; i++ )
      {
        int offset = slopFactor - i;
        offset = remainingFactor - offset;
        globalInterpolants[i] = inputBuffer->getReductionTextureRect(
          offset, remainingExtent + offset, 0, otherExtent, dim );
      }
      globalInterpolants[slopFactor] = slopBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim );
      outputRect = slopBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );
  
      executeReductionTechnique( slopFactor+1 );
      ioState.slopCount++;
    }
  }

  ioState.whichBuffer = nextBuffer;

#ifdef BROOK_DX9_TRACE_REDUCE
  dumpReductionState( ioState );
#endif
}

void DX9Kernel::executeSlopStep( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::executeSlopStep")
  DX9LOG(3) << "Slop Step";

  if( ioState.slopCount == 0 ) return;

  int dim = ioState.currentDimension;
  int outputExtent = ioState.currentExtents[dim];
  int otherExtent = ioState.currentExtents[1-dim];

  // bind the buffers and rectangles needed
  DX9Texture* slopBuffer = ioState.slopBuffer;
  DX9Texture* inputBuffer = ioState.reductionBuffers[ioState.whichBuffer];
  DX9Texture* outputBuffer = inputBuffer; // TIM: not future-proof

  globalSamplers[0] = inputBuffer->getTextureHandle();
  globalSamplers[1] = slopBuffer->getTextureHandle();
  globalOutputs[0] = outputBuffer->getSurfaceHandle();


  globalInterpolants[0] = inputBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim);
  globalInterpolants[1] = slopBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim );
  outputRect = outputBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );

  // bind the 2-argument reduction function
  executeReductionTechnique( 2 );

  ioState.slopCount = 0;

#ifdef BROOK_DX9_TRACE_REDUCE
  dumpReductionState( ioState );
#endif
}

void DX9Kernel::endReduction( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::endReduction")
  int outputWidth = ioState.targetExtents[0];
  int outputHeight = ioState.targetExtents[1];

  DX9Texture* inputBuffer = ioState.reductionBuffers[ioState.whichBuffer];
  DX9Texture* outputBuffer = ioState.outputTexture;

  device->SetPixelShader( runtime->getPassthroughPixelShader()->getHandle() );
  device->SetTexture( 0, inputBuffer->getTextureHandle() );
  device->SetRenderTarget( 0, outputBuffer->getSurfaceHandle() );

  inputTextureRects.push_back( inputBuffer->getReductionTextureRect( 0, outputWidth, 0, outputHeight, 0 ) );
  outputRect = outputBuffer->getReductionSurfaceRect( 0, outputWidth, 0, outputHeight, 0 );
  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
  clearInputs();

  // clean up
  HRESULT result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );

  ioState.outputTexture->markCachedDataChanged();

  DX9LOG(3) << "************ Result *************";
  dumpReductionBuffer( outputBuffer, 1, 1 );

  clearArguments();
}

void DX9Kernel::dumpReductionState( ReductionState& ioState )
{
  DX9LOG(3) << "********************* Reduction Dump *************";
  int dim = ioState.currentDimension;
  int buffer = ioState.whichBuffer;
 
  if( buffer == -1 )
  {
    DX9LOG(3) << "Input";
    dumpReductionBuffer( ioState.inputTexture,
      ioState.currentExtents[0], ioState.currentExtents[1] );
  }
  else
  {
    DX9LOG(3) << "Buffer";
    ioState.reductionBuffers[buffer]->markCachedDataChanged();
    dumpReductionBuffer( ioState.reductionBuffers[buffer],
      ioState.currentExtents[0], ioState.currentExtents[1] );
  }

  if( ioState.slopCount )
  {
    int slopExtents[2];
    slopExtents[0] = ioState.currentExtents[0];
    slopExtents[1] = ioState.currentExtents[1];
    slopExtents[dim] = ioState.targetExtents[dim];

    DX9LOG(3) << "Slop";
    ioState.slopBuffer->markCachedDataChanged();
    dumpReductionBuffer( ioState.slopBuffer, slopExtents[0], slopExtents[1] );
  }
}

void DX9Kernel::dumpReductionBuffer( DX9Texture* inBuffer, int inWidth, int inHeight )
{
  static float4* data = new float4[2048*2048];

  int bufferWidth = inBuffer->getWidth();
  int bufferHeight = inBuffer->getHeight();

  int w = inWidth;
  int h = inHeight;
  if( w == 0 )
    w = bufferWidth;
  if( h == 0 )
    h = bufferHeight;

  inBuffer->getData( (float*)data, sizeof(float4), bufferWidth*bufferHeight );

  float4* line = data;
  for( int y = 0; y < h; y++ )
  {
    float4* pixel = line;
    for( int x = 0; x < w; x++ )
    {
      if( x > 0 && x % 5 == 0 )
        DX9LOGPRINT(3) << "\n\t";

      float4 value = *pixel++;
      DX9LOGPRINT(3) << "{" << value.x
        << " " << value.y
        << " " << value.z
        << " " << value.w << "}";
    }
    line += bufferWidth;
    DX9LOGPRINT(3) << std::endl;
  }
}
