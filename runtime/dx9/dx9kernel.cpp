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
  : runtime(inRuntime),
  device(NULL),
  pixelShader(NULL)
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
    const char* programString = (const char*)inSource[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      if( programString != NULL )
        return initialize( programString );
    }

    i += 2;
  }

  DX9Warn("Unable to find pixel shader 2.0 code.");
  return false;
}

bool DX9Kernel::initialize( const char* source )
{
  pixelShader = DX9PixelShader::create( runtime, source );
  if( pixelShader == NULL )
  {
    DX9Warn( "Failed to create kernel's pixel shader" );
    return false;
  }

  // look for our annotations...
  std::string s = source;

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
    argumentUsesIndexof[i] = (indexofHint == 'i');
  }

  // initialize all the rects, just in case
  outputRect = DX9Rect(0,0,0,0);
  for( i = 0; i < 8; i++ )
  {
    inputRects[i] = DX9Rect(0,0,0,0);
    inputTextures[i] = NULL;
  }

  return true;
}

DX9Kernel::~DX9Kernel()
{
  if( pixelShader != NULL )
    delete pixelShader;
  if( device != NULL )
    device->Release();
}

void DX9Kernel::PushStream(Stream *s) {
  int arg = argumentIndex++;
  DX9Stream* stream = (DX9Stream*)s;

  // reduction stream is always the first/last/only stream pushed
  inputReductionStream = stream;
  inputReductionStreamSamplerIndex = argumentSamplerIndex;
  inputReductionStreamTexCoordIndex = argumentTexCoordIndex;

  PushSampler( stream );
  PushTexCoord( stream->getInputRect() );
  if( argumentUsesIndexof[arg] )
    PushConstantImpl( stream->getIndexofConstant() );
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
  argumentReductionIndex++;
  outputReductionData = val;
  outputReductionType = type;

  outputReductionVarSamplerIndex = argumentSamplerIndex++;
  outputReductionVarTexCoordIndex = argumentTexCoordIndex++;
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
  PushConstantImpl( stream->getGatherConstant() );
  PushSampler( stream );
}

void DX9Kernel::PushOutput(Stream *s) {
  DX9Trace("PushOutput");
  int arg = argumentIndex++;
  argumentOutputIndex++;

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DSurface9* surfaceHandle = stream->getSurfaceHandle();

  outputStream = stream;
  outputSurface = surfaceHandle;
  outputRect = stream->getOutputRect();

  if( argumentUsesIndexof[arg] )
  {
    PushConstantImpl( stream->getIndexofConstant() );
    PushTexCoord( stream->getInputRect() );
  }
}

void DX9Kernel::Map() {
  DX9Trace("Map");
  HRESULT result;

  int samplerCount = argumentSamplerIndex;
  int texCoordCount = argumentTexCoordIndex;
  int constantCount = argumentConstantIndex;
  int outputCount = argumentOutputIndex;
  int reductionCount = argumentReductionIndex;
  ClearInputs();

  DX9VertexShader* vertexShader = runtime->getPassthroughVertexShader();

  result = device->SetRenderTarget( 0, outputSurface );
  DX9AssertResult( result, "SetRenderTarget failed" );

  result = device->BeginScene();
  DX9AssertResult( result, "BeginScene failed"  );

  result = device->SetPixelShader( pixelShader->getHandle() );
  DX9AssertResult( result, "SetPixelShader failed" );
  result = device->SetVertexShader( vertexShader->getHandle() );
  DX9AssertResult( result, "SetVertexShader failed" );
  int i;
  for( i = 0; i < samplerCount; i++ )
  {
    inputStreams[i]->validateGPUData();
    result = device->SetTexture( i, inputTextures[i] );
    DX9AssertResult( result, "SetTexture failed" );
  }

  // TIM: TODO: set up workspace constant

  for( i = 0; i < constantCount; i++ )
  {
    result = device->SetPixelShaderConstantF( i+kBaseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9AssertResult( result, "SetPixelShaderConstantF failed" );
  }

  runtime->execute( outputRect, inputRects );

  outputStream->markGPUDataChanged();

  result = device->EndScene();
  DX9AssertResult( result, "EndScene failed" );
}

void DX9Kernel::Reduce() {
  DX9Trace("Reduce");

  if( outputReductionType == __BRTSTREAM )
  {
    Stream* outputStreamBase = *((const __BRTStream*)outputReductionData);
    DX9Stream* outputStream = (DX9Stream*)outputStreamBase;
    DX9Texture* outputTexture = outputStream->getTexture();
    ReduceToStream( outputTexture );
  }
  else
  {
    DX9Texture* outputTexture = runtime->getReductionTargetBuffer();
    ReduceToStream( outputTexture );

    float4 reductionResult;
    outputTexture->getData( (float*)&reductionResult );
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

void DX9Kernel::PushSampler( DX9Stream* s )
{
  int samplerIndex = argumentSamplerIndex++;
  DX9Trace("PushSampler[%d]",samplerIndex);
  IDirect3DTexture9* texture = s->getTextureHandle();

  inputStreams[samplerIndex] = s;
  inputTextures[samplerIndex] = texture;
}

void DX9Kernel::PushTexCoord( const DX9FatRect& r )
{
  int textureUnit = argumentTexCoordIndex++;
  DX9Trace("PushTexCoord[%d]",textureUnit);
  inputRects[textureUnit] = r;
}

void DX9Kernel::PushConstantImpl(const float4 &val) {
  int arg = argumentConstantIndex++;
  int constantIndex = arg;
  DX9Trace("PushConstant[%d] < %3.2f, %3.2f, %3.2f, %3.2f >",
    constantIndex+kBaseConstantIndex, val.x, val.y, val.z, val.w );
  inputConstants[constantIndex] = val;
}

void DX9Kernel::ClearInputs()
{
  argumentIndex = 0;
  argumentSamplerIndex = 0;
  argumentTexCoordIndex = 0;
  argumentConstantIndex = 0;
  argumentOutputIndex = 0;
  argumentReductionIndex = 0;
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
  int inputWidth = inputStream->getWidth();
  int inputHeight = inputStream->getHeight();

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

  inputRects[0] = reductionBuffer->getTextureSubRect(
    kSideOffsets[currentSide], 0,
    kSideOffsets[currentSide] + outputWidth, outputHeight );
  outputRect = outputBuffer->getSurfaceSubRect( 0, 0, outputWidth, outputHeight );
  runtime->execute( outputRect, inputRects );

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
        inputRects[0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
          slopBufferOffset, 0, remainingExtent+slopBufferOffset, remainingOtherExtent, slopBufferStride, 1, dim );
        outputRect = reductionBuffer->getReductionSurfaceSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, dim );
        runtime->execute( outputRect, inputRects );
        BindReductionOperationState();
      }
      else // composite with existing one...
      {
        inputRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
          slopBufferOffset, 0, remainingExtent+slopBufferOffset, remainingOtherExtent, slopBufferStride, 1, dim );
        inputRects[tex1] = reductionBuffer->getReductionTextureSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, 1, 1, dim );
        outputRect = reductionBuffer->getReductionSurfaceSubRect( 0, kDX9ReductionBufferHeight/2,
          0, 0, outputExtent, remainingOtherExtent, dim );
        runtime->execute( outputRect, inputRects );
      }
    }

    int collapseGroupExtent = remainingFactor & ~1;
    remainingFactor /= 2;
    int currentExtent = remainingExtent;
    int collapseExtent = remainingFactor*outputExtent;
    remainingExtent = collapseExtent;

    // we have dealt with the slop, so now we just collapse the rest by a factor of two
    inputRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      0, 0, currentExtent, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    inputRects[tex1] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      1, 0, currentExtent+1, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    currentSide = 1-currentSide;
    outputRect = reductionBuffer->getReductionSurfaceSubRect( kSideOffsets[currentSide], 0,
      0, 0, remainingExtent, remainingOtherExtent, dim );
    runtime->execute( outputRect, inputRects );
  }

  DX9Assert( remainingExtent == outputExtent, "Failed to reduce by the right amount!!!" );

#if defined(BROOK_DX9_TRACE_REDUCE)
  DumpReduceDimensionState( currentSide, outputExtent, remainingExtent, remainingOtherExtent, slopBufferCount, dim );
#endif

  // if we have slop buffers, composite them into place
  if( slopBufferCount != 0 )
  {
    inputRects[tex0] = reductionBuffer->getReductionTextureSubRect( kSideOffsets[currentSide], 0,
      0, 0, outputExtent, remainingOtherExtent, 1, 1, dim ); // TIM: stride is not used here... bad
    inputRects[tex1] = reductionBuffer->getReductionTextureSubRect( 0, kDX9ReductionBufferHeight/2,
      0, 0, outputExtent, remainingOtherExtent, 1, 1, dim );
    outputRect = reductionBuffer->getReductionSurfaceSubRect( kSideOffsets[currentSide], 0,
      0, 0, outputExtent, remainingOtherExtent, dim );
    runtime->execute( outputRect, inputRects );

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
  int constantCount = argumentConstantIndex;
  int outputCount = argumentOutputIndex;
  int reductionCount = argumentReductionIndex;

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

  int inputWidth = inStream->getWidth();
  int inputHeight = inStream->getHeight();

  inStream->validateGPUData();
  result = device->SetTexture( 0, inStream->getTextureHandle() );
  DX9AssertResult( result, "SetTexture failed" );
  inputRects[0] = inStream->getTextureSubRect( 0, 0, inputWidth, inputHeight );
  outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, inputWidth, inputHeight );
  runtime->execute( outputRect, inputRects );
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
  int samplerCount = argumentSamplerIndex;
  DX9Texture* reductionBuffer = runtime->getReductionBuffer();

  result = device->SetPixelShader( pixelShader->getHandle() );
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
  runtime->getReductionBuffer()->getData( (float*)data );
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
