#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"
#include "dx9texture.hpp"
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
  : runtime(inRuntime), hasPushedOutputIndexof(false),
  device(NULL), streamShapeMismatch(false), mustMatchShapeStream(NULL)
{
  device = inRuntime->getDevice();
  device->AddRef();
}

bool DX9Kernel::initialize( const void* inSource[] )
{
  ClearInputs();
  
  int i = 0;
  while( inSource[i] != NULL )
  {
    const char* nameString = (const char*)inSource[i];
    const char** programStrings = (const char**)inSource[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      if( programStrings != NULL )
        return initialize( programStrings );
    }

    i += 2;
  }

  DX9WARN << "Unable to find pixel shader 2.0 code.";
  return false;
}

bool DX9Kernel::initialize( const char** inProgramStrings )
{
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

  // initialize the output rects, just in case
  outputRect = DX9Rect(0,0,0,0);
  return true;
}

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
}

DX9Kernel::~DX9Kernel()
{
  int p,passCount = (int)standardPasses.size();
  for( p = 0; p < passCount; p++ )
    delete standardPasses[p].pixelShader;
  passCount = (int)fullTranslationPasses.size();
  for( p = 0; p < passCount; p++ )
    delete fullTranslationPasses[p].pixelShader;

  if( device != NULL )
    device->Release();
}

void DX9Kernel::PushStream(Stream *s) {
  DX9PROFILE("DX9Kernel::PushStream")
  int arg = argumentIndex++;
  DX9Stream* stream = (DX9Stream*)s;

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
}

void DX9Kernel::PushIter(class Iter * v)
{
  DX9PROFILE("DX9Kernel::PushIter")
  int arg = argumentIndex++;
  DX9Iter* iterator = (DX9Iter*)v;
  PushTexCoord( iterator->getRect() );
}

void DX9Kernel::PushReduce(void * val, __BRTStreamType type) {
  DX9PROFILE("DX9Kernel::PushReduce")
  int arg = argumentIndex++;
  DX9LOG(1) << "PushReduce";
  
  outputReductionDatas.push_back(val);
  outputReductionTypes.push_back(type);

  outputReductionVarSamplerIndex = (int)inputTextures.size();
  outputReductionVarTexCoordIndex = (int)inputTextureRects.size();

  inputTextures.push_back(NULL);
  inputTextureRects.push_back(DX9Rect(0,0,0,0));
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
  argumentIndex++;
  PushConstantImpl(val);
}

void DX9Kernel::PushGatherStream(Stream *s) {
  DX9PROFILE("DX9Kernel::PushGatherStream")
  argumentIndex++;
  DX9Stream* stream = (DX9Stream*)s;

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
}

void DX9Kernel::PushOutput(Stream *s) {
  DX9PROFILE("DX9Kernel::PushOutput")
  DX9LOG(1) << "PushOutput";
  int arg = argumentIndex++;

  DX9Stream* stream = (DX9Stream*)s;

  if( outputStreams.size() == 0 )
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
}

void DX9Kernel::mapPass( const DX9Kernel::Pass& inPass )
{
  DX9PROFILE("DX9Kernel::mapPass")
  HRESULT result;
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

}

void DX9Kernel::Map() {
  DX9PROFILE("DX9Kernel::Map")
  DX9LOG(1) << "Map";

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
}

void DX9Kernel::Reduce() {
  DX9PROFILE("DX9Kernel::Reduce")
  DX9LOG(1) << "Reduce";

  DX9Assert( outputReductionTypes.size() == 1,
    "Must have one and only one reduction output for now" );

  __BRTStreamType outputReductionType = outputReductionTypes[0];
  void* outputReductionData = outputReductionDatas[0];

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

void DX9Kernel::PushSamplers( DX9Stream* s )
{
  DX9PROFILE("DX9Kernel::PushSamplers")
  int textureCount = s->getSubstreamCount();
  for( int i = 0; i < textureCount; i++ )
  {
    inputTextures.push_back( s->getIndexedTextureHandle(i) );
  }
}

void DX9Kernel::PushTexCoord( const DX9FatRect& r )
{
  DX9PROFILE("DX9Kernel::PushTexCoord")
  inputTextureRects.push_back(r);
}

int DX9Kernel::PushConstantImpl(const float4 &val) {
  int result = (int)inputConstants.size();
  inputConstants.push_back(val);
  return result;
}

void DX9Kernel::ClearInputs()
{
  DX9PROFILE("DX9Kernel::ClearInputs")
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

  DX9Stream* inputStream = inputReductionStream;
  int tex0 = inputReductionStreamTexCoordIndex;
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
  }
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
  state.leftTextureUnit = tex0;
  state.rightTextureUnit = tex1;
  state.leftSampler = sampler0;
  state.rightSampler = sampler1;
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

void DX9Kernel::bindReductionPassShader( int inFactor )
{
  DX9PROFILE("DX9Kernel::bindReductionPassShader")
  ReductionPass& reductionPass = reductionPasses[ inFactor - 2 ];
  DX9Assert( reductionPass.standardPasses.size() == 1, "Must have single-pass completion for reductions" );
  HRESULT result = device->SetPixelShader( reductionPass.standardPasses[0].pixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );
}

void DX9Kernel::beginReduction( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::beginReduction")
  HRESULT result;

  ioState.inputTexture->validateCachedData();

  // set up
  result = device->BeginScene();
  DX9AssertResult( result, "BeginScene failed" );

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
}

void DX9Kernel::executeReductionStep( ReductionState& ioState )
{
  DX9PROFILE("DX9Kernel::executeReductionStep")
  DX9LOG(3) << "Reduction Step";

  HRESULT result;

  int dim = ioState.currentDimension;
  int remainingFactor = ioState.currentExtents[dim] / ioState.targetExtents[dim];
  int remainingExtent = ioState.currentExtents[dim];
  int outputExtent = ioState.targetExtents[dim];
  int otherExtent = ioState.currentExtents[1-dim];
  int tex0 = ioState.leftTextureUnit;
  int tex1 = ioState.rightTextureUnit;

  int reductionPassCount = reductionPasses.size();
  for( int r = reductionPassCount-1; r >= 0; r-- )
  {
    int passFactor = reductionPasses[r].reductionFactor;

    int quotient = remainingFactor / passFactor;
    int remainder = remainingFactor % passFactor;

    if( quotient == 0 ) continue;
    if( quotient == 1 || remainder <= 1 ) break;
  }
  int reductionFactor = reductionPasses[r].reductionFactor;
  int slopFactor = (remainingFactor % reductionFactor);

  int resultExtents[2];
  resultExtents[0] = ioState.currentExtents[0];
  resultExtents[1] = ioState.currentExtents[1];
  resultExtents[dim] = outputExtent * (remainingFactor / reductionFactor);

  // Set up the proper pixel shader
  bindReductionPassShader( reductionFactor );

  // bind the proper textures as the shader inputs
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

  result = device->SetTexture( ioState.leftSampler, inputBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  result = device->SetTexture( ioState.rightSampler, inputBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  result = device->SetRenderTarget( 0, outputBuffer->getSurfaceHandle() );
  DX9AssertResult( result, "SetRenderTarget failed" );

  if( (int)inputTextureRects.size() < reductionFactor )
    inputTextureRects.resize( reductionFactor );

  for( int i = 0; i < reductionFactor; i++ )
  {
    inputTextureRects[i] = inputBuffer->getReductionTextureRect(
      i, remainingExtent+i, 0, otherExtent, dim );
  }
  int newExtent = resultExtents[dim];
  ioState.currentExtents[dim] = newExtent;
  outputRect = outputBuffer->getReductionSurfaceRect(
    0, newExtent, 0, otherExtent, dim );

  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

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

    if( ioState.slopCount == 0 )
    {
      if( slopFactor > 1 )
      {
        bindReductionPassShader( slopFactor );
        device->SetTexture( ioState.leftSampler, inputBuffer->getTextureHandle() );
        device->SetTexture( ioState.rightSampler, inputBuffer->getTextureHandle() );

        for( int i = 0; i < slopFactor; i++ )
        {
          int offset = slopFactor - i;
          offset = remainingFactor - offset;
          inputTextureRects[i] = inputBuffer->getReductionTextureRect(
            offset, remainingExtent + offset, 0, otherExtent, dim );
        }
      }
      else
      {
        device->SetPixelShader( runtime->getPassthroughPixelShader()->getHandle() );
        device->SetTexture( 0, inputBuffer->getTextureHandle() );

        int offset = remainingFactor-1;
        inputTextureRects[0] = inputBuffer->getReductionTextureRect( offset, remainingExtent+offset, 0, otherExtent, dim );
      }
    }
    else
    {
      bindReductionPassShader( slopFactor+1 ); // must exist for weird reasons... :)
      device->SetTexture( ioState.leftSampler, inputBuffer->getTextureHandle() );
      device->SetTexture( ioState.rightSampler, slopBuffer->getTextureHandle() );

      for( int i = 0; i < slopFactor; i++ )
      {
        int offset = slopFactor - i;
        offset = remainingFactor - offset;
        inputTextureRects[i] = inputBuffer->getReductionTextureRect(
          offset, remainingExtent + offset, 0, otherExtent, dim );
      }
      inputTextureRects[slopFactor] = slopBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim );
    }
    device->SetRenderTarget( 0, slopBuffer->getSurfaceHandle() );
    outputRect = slopBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );
    runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
    ioState.slopCount++;
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

  HRESULT result;

  if( ioState.slopCount == 0 ) return;

  int dim = ioState.currentDimension;
  int outputExtent = ioState.currentExtents[dim];
  int otherExtent = ioState.currentExtents[1-dim];
  int tex0 = ioState.leftTextureUnit;
  int tex1 = ioState.rightTextureUnit;

  // bind the 2-argument reduction function
  bindReductionPassShader( 2 );

  // bind the buffers and rectangles needed
  DX9Texture* slopBuffer = ioState.slopBuffer;
  DX9Texture* inputBuffer = ioState.reductionBuffers[ioState.whichBuffer];
  DX9Texture* outputBuffer = inputBuffer; // TIM: not future-proof

  result = device->SetTexture( ioState.leftSampler, inputBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  result = device->SetTexture( ioState.rightSampler, slopBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  result = device->SetRenderTarget( 0, outputBuffer->getSurfaceHandle() );
  DX9AssertResult( result, "SetRenderTarget failed" );

  inputTextureRects[tex0] = inputBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim);
  inputTextureRects[tex1] = slopBuffer->getReductionTextureRect( 0, outputExtent, 0, otherExtent, dim );
  outputRect = outputBuffer->getReductionSurfaceRect( 0, outputExtent, 0, otherExtent, dim );

  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

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

  inputTextureRects[0] = inputBuffer->getReductionTextureRect( 0, outputWidth, 0, outputHeight, 0 );
  outputRect = outputBuffer->getReductionSurfaceRect( 0, outputWidth, 0, outputHeight, 0 );
  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

  // clean up
  HRESULT result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );

  ioState.outputTexture->markCachedDataChanged();

  DX9LOG(3) << "************ Result *************";
  dumpReductionBuffer( outputBuffer, 1, 1 );

  ClearInputs();
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
