#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"
#include "dx9texture.hpp"
#include <string>

using namespace brook;

static const char* PIXEL_SHADER_NAME_STRING = "ps20";

DX9Kernel* DX9Kernel::create( DX9RunTime* inRuntime, const void* inSource[] )
{
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

  DX9Warn("Unable to find pixel shader 2.0 code.");
  return false;
}

bool DX9Kernel::initialize( const char** inProgramStrings )
{
  // must have least one shader
  if( *inProgramStrings == NULL )
  {
    DX9Warn( "No attached ps20 pixel shaders found" );
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
      DX9Warn( "Failed to create a kernel pass pixel shader" );
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

    if( !addressTrans )
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

DX9Kernel::~DX9Kernel()
{
  int passCount = (int)standardPasses.size();
  for( int p = 0; p < passCount; p++ )
    delete standardPasses[p].pixelShader;
  passCount = (int)fullTranslationPasses.size();
  for( int p = 0; p < passCount; p++ )
    delete fullTranslationPasses[p].pixelShader;

  if( device != NULL )
    device->Release();
}

void DX9Kernel::PushStream(Stream *s) {
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
  int arg = argumentIndex++;
  DX9Iter* iterator = (DX9Iter*)v;
  PushTexCoord( iterator->getRect() );
}

void DX9Kernel::PushReduce(void * val, __BRTStreamType type) {
  int arg = argumentIndex++;
  DX9Trace("PushReduce");
  
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
  argumentIndex++;
  PushConstantImpl(val);
}

void DX9Kernel::PushGatherStream(Stream *s) {
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
  DX9Trace("PushOutput");
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

  // TIM: TODO: set up workspace constant

  int baseConstantIndex = kBaseConstantIndex;
  if( runtime->isAddressTranslationOn() )
  {
    DX9Stream* outputStream = outputStreams[0];

    float4 outputConstant = outputStream->getATOutputConstant();
    float4 hackConstant(1,1,1,1);

    result = device->SetPixelShaderConstantF( 0, (float*)&(outputConstant), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
    result = device->SetPixelShaderConstantF( 1, (float*)&(hackConstant), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );

    baseConstantIndex = 2;
  }

  for( i = 0; i < constantCount; i++ )
  {
    result = device->SetPixelShaderConstantF( i+baseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }

  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

  result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );

}

void DX9Kernel::Map() {
  DX9Trace("Map");

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
  DX9Trace("Reduce");

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
  int textureCount = s->getSubstreamCount();
  for( int i = 0; i < textureCount; i++ )
  {
    inputTextures.push_back( s->getIndexedTextureHandle(i) );
  }
}

void DX9Kernel::PushTexCoord( const DX9FatRect& r )
{
  inputTextureRects.push_back(r);
}

int DX9Kernel::PushConstantImpl(const float4 &val) {
  int result = (int)inputConstants.size();
  inputConstants.push_back(val);
  return result;
}

void DX9Kernel::ClearInputs()
{
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
  HRESULT result;

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
  int inputWidth = inputStream->getTextureWidth();
  int inputHeight = inputStream->getTextureHeight();

  DX9Assert( (inputWidth <= kDX9ReductionBufferWidth/2) && (inputHeight <= kDX9ReductionBufferHeight/2),
    "A stream to be reduced was too large for the reduction buffer." );

  DX9Texture* reductionBuffer = runtime->getReductionBuffer();

  result = device->BeginScene();
  DX9AssertResult( result, "BeginScene failed" );

  BindReductionBaseState();
  CopyStreamIntoReductionBuffer( inputStream );

  BindReductionOperationState();

  int currentSide = 0;
  int extents[2] = {inputWidth, inputHeight};
  static const int kSideOffsets[2] = {0,kDX9ReductionBufferWidth/2};
  ReduceDimension( currentSide, tex0, tex1, 2, 0, outputWidth, extents );
  ReduceDimension( currentSide, tex0, tex1, 2, 1, outputHeight, extents );

  // finally copy the data out to the output buffer
  BindReductionPassthroughState();
  result = device->SetRenderTarget( 0, outputBuffer->getSurfaceHandle() );
  DX9AssertResult( result, "SetRenderTarget failed" );

  inputTextureRects[0] = reductionBuffer->getTextureSubRect(
    kSideOffsets[currentSide], 0,
    kSideOffsets[currentSide] + outputWidth, outputHeight );
  outputRect = outputBuffer->getSurfaceSubRect( 0, 0, outputWidth, outputHeight );
  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

  result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );

  outputBuffer->markCachedDataChanged();

  ClearInputs();
}

void DX9Kernel::ReduceDimension( int& ioReductionBufferSide,
      int inReductionTex0, int inReductionTex1,
      int inDimensionCount, int inDimensionToReduce,
      int inExtentToReduceTo, int* ioRemainingExtents )
{
  // simple special case: reducing to a single value
/*  if( inExtentToReduceTo == 1 )
  {
    ReduceDimensionToOne( ioReductionBufferSide, inReductionTex0, inReductionTex1,
      inDimensionCount, inDimensionToReduce, ioRemainingExtents );
    return;
  }*/

  // general case, we need to deal with all kinds of ugliness :)
  int currentSide = ioReductionBufferSide;
  int tex0 = inReductionTex0;
  int tex1 = inReductionTex1;
  int dim = inDimensionToReduce;
  int inputExtent = ioRemainingExtents[ dim ];
  int remainingOtherExtent = ioRemainingExtents[ 1-dim ]; // TIM: assumes 2D
  int outputExtent = inExtentToReduceTo;
  int reductionFactor = inputExtent / outputExtent;
  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  static const int kSideOffsets[2] = {0,kDX9ReductionBufferWidth/2};

  if( inputExtent == outputExtent ) return;

  DX9Assert( outputExtent < inputExtent, "Output extent must be less than or equal to input extent for reduction." );
  DX9Assert( (inputExtent % outputExtent) == 0, "Output extent must evenly divide input extent for reduction" );

  int remainingFactor = reductionFactor;
  int remainingExtent = inputExtent;
  int slopBufferCount = 0;
  while( remainingFactor > 1 )
  {
#if defined(BROOK_DX9_TRACE_REDUCE)
    DumpReduceDimensionState( currentSide, outputExtent, remainingExtent, remainingOtherExtent, slopBufferCount, dim );
#endif

    if( remainingFactor & 1 ) // odd factor
    {
      // we introduce another slop buffer
      int slopBufferOffset = remainingFactor-1;
      int slopBufferStride = remainingFactor;
      slopBufferCount++;

      if( slopBufferCount == 1 ) // first one...
      {
        BindReductionPassthroughState();
        inputTextureRects[0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
          slopBufferOffset, 0, remainingExtent+slopBufferOffset, remainingOtherExtent, slopBufferStride, 1, dim );
        outputRect = reductionBuffer->getReductionSurfaceSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, dim );
        runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
        BindReductionOperationState();
      }
      else // composite with existing one...
      {
        inputTextureRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
          slopBufferOffset, 0, remainingExtent+slopBufferOffset, remainingOtherExtent, slopBufferStride, 1, dim );
        inputTextureRects[tex1] = reductionBuffer->getReductionTextureSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, 1, 1, dim );
        outputRect = reductionBuffer->getReductionSurfaceSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, dim );
          runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
      }
    }

    int collapseGroupExtent = remainingFactor & ~1;
    remainingFactor /= 2;
    int currentExtent = remainingExtent;
    int collapseExtent = remainingFactor*outputExtent;
    remainingExtent = collapseExtent;

    // we have dealt with the slop, so now we just collapse the rest by a factor of two
    inputTextureRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      0, 0, currentExtent, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    inputTextureRects[tex1] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      1, 0, currentExtent+1, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    currentSide = 1-currentSide;
    outputRect = reductionBuffer->getReductionSurfaceSubRect( kSideOffsets[currentSide], 0,
      0, 0, remainingExtent, remainingOtherExtent, dim );
    runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
  }

  DX9Assert( remainingExtent == outputExtent, "Failed to reduce by the right amount!!!" );

#if defined(BROOK_DX9_TRACE_REDUCE)
  DumpReduceDimensionState( currentSide, outputExtent, remainingExtent, remainingOtherExtent, slopBufferCount, dim );
#endif

  // if we have slop buffers, composite them into place
  if( slopBufferCount != 0 )
  {
    inputTextureRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      0, 0, outputExtent, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    inputTextureRects[tex1] = reductionBuffer->getReductionTextureSubRect( 0, kDX9ReductionBufferHeight/2,
      0, 0, outputExtent, remainingOtherExtent, 1, 1, dim );
    outputRect = reductionBuffer->getReductionSurfaceSubRect( kSideOffsets[currentSide], 0,
      0, 0, outputExtent, remainingOtherExtent, dim );
    runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );

#if defined(BROOK_DX9_TRACE_REDUCE)
    DumpReduceDimensionState( currentSide, outputExtent, remainingExtent, remainingOtherExtent, 0, dim );
#endif
  }

  ioReductionBufferSide = currentSide;
  ioRemainingExtents[dim] = outputExtent;
}

void DX9Kernel::BindReductionBaseState()
{
  HRESULT result;
  int constantCount = (int)inputConstants.size();
  int outputCount = (int)outputStreams.size();
  int reductionCount = (int)outputReductionTypes.size();

  // inspect the input stuff:
  DX9Assert( reductionCount == 1, "Number of 'reduce' arguments was not 1." );
  DX9Assert( outputCount == 0, "'out' streams found in reduce function." );

  DX9VertexShader* passthroughVertexShader = runtime->getPassthroughVertexShader();
  result = device->SetVertexShader( passthroughVertexShader->getHandle() );
  DX9AssertResult( result, "SetVertexShader failed" );

  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  result = device->SetRenderTarget( 0, reductionBuffer->getSurfaceHandle() );
  DX9AssertResult( result, "SetRenderTarget failed" );

  // TODO: workspace constant
  for( int i = 0; i < constantCount; i++ )
  {
    result = device->SetPixelShaderConstantF( i+kBaseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }
}

void DX9Kernel::CopyStreamIntoReductionBuffer( DX9Stream* inStream )
{
  HRESULT result;
  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  DX9PixelShader* passthroughPixelShader = runtime->getPassthroughPixelShader();
  result = device->SetPixelShader( passthroughPixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );

  DX9Assert( inStream->getSubstreamCount() == 1,
    "Only one-field streams can be reduced for now" );

  int inputWidth = inStream->getTextureWidth();
  int inputHeight = inStream->getTextureHeight();

  inStream->validateGPUData();
  result = device->SetTexture( 0, inStream->getIndexedTextureHandle(0) );
  DX9AssertResult( result, "SetTexture failed" );
  inputTextureRects[0] = inStream->getTextureSubRect( 0, 0, inputWidth, inputHeight );
  outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, inputWidth, inputHeight );
  runtime->execute( outputRect, (int)inputTextureRects.size(), &inputTextureRects[0] );
}

void DX9Kernel::BindReductionPassthroughState()
{
  HRESULT result;
  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  DX9PixelShader* passthroughPixelShader = runtime->getPassthroughPixelShader();

  // first pass - just copy the data into the reduction buffer...
  // this step *could* be elliminated in the future
  result = device->SetPixelShader( passthroughPixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );

  result = device->SetTexture( 0, reductionBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
}

void DX9Kernel::BindReductionOperationState()
{
  HRESULT result;
  int samplerCount = (int)inputTextures.size();
  DX9Texture* reductionBuffer = runtime->getReductionBuffer();

  DX9Assert( standardPasses.size() == 1, "Only a single output allowed for reductions right now" );

  result = device->SetPixelShader( standardPasses[0].pixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );

  int sampler0 = inputReductionStreamSamplerIndex;
  int sampler1 = outputReductionVarSamplerIndex;

  for( int i = 0; i < samplerCount; i++ )
  {
    if( i == sampler0 || i == sampler1 ) continue;
    inputStreams[i]->validateGPUData();
    result = device->SetTexture( i, inputTextures[i] );
  DX9AssertResult( result, "SetTexture failed" );
  }

  result = device->SetTexture( sampler0, reductionBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  result = device->SetTexture( sampler1, reductionBuffer->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
}

void DX9Kernel::DumpReductionBuffer( int xOffset, int yOffset, int axisMin, int otherMin, int axisMax, int otherMax, int dim )
{
  static float4* data = new float4[kDX9ReductionBufferWidth*kDX9ReductionBufferHeight];
  runtime->getReductionBuffer()->markCachedDataChanged();
  runtime->getReductionBuffer()->getData( (float*)data, sizeof(float4), kDX9ReductionBufferWidth*kDX9ReductionBufferHeight );
  int xMin, yMin, xMax, yMax;
  if( dim == 0 )
  {
    xMin = axisMin; xMax = axisMax;
    yMin = otherMin; yMax = otherMax;
  }
  else
  {
    yMin = axisMin; yMax = axisMax;
    xMin = otherMin; xMax = otherMax;
  }
  int count = (xMax-xMin)*(yMax-yMin);
  if( count == 0 ) return;

  for( int i = yMin; i < yMax; i++ )
  {
    for( int j = xMin; j < xMax; j++ )
    {
        float4 value = data[(i+yOffset)*kDX9ReductionBufferWidth + (j+xOffset)];
        DX9Print( "{%6.3f %6.3f %6.3f %6.3f} ", value.x, value.y, value.z, value.w );
    }
    DX9Print( "\n" );
  }
}

void DX9Kernel::DumpReduceDimensionState(
  int currentSide, int outputExtent, int remainingExtent, int remainingOtherExtent, int slopBufferCount, int dim )
{
  DX9Trace("ReduceDim[%d] - remain = %d, slopCount = %d", dim, remainingExtent, slopBufferCount );

  DX9Print("NormalBuffer:\n");
  DumpReductionBuffer( (currentSide == 0) ? 0 : kDX9ReductionBufferWidth/2, 0,
    0, 0, remainingExtent, remainingOtherExtent, dim );
  if( slopBufferCount > 0 )
  {
    DX9Print("Slop Buffer:\n");
    DumpReductionBuffer( 0, kDX9ReductionBufferHeight/2,
      0, 0, outputExtent, remainingOtherExtent, dim );
  }
}
