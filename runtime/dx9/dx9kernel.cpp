#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"
#include "dx9texture.hpp"

using namespace brook;

static const char* PIXEL_SHADER_NAME_STRING = "ps20";

DX9Kernel::DX9Kernel(DX9RunTime* runtime, const void* source[])
  : runtime(runtime)
{
  DX9Trace("DX9Kernel::DX9Kernel");

  ClearInputs();
  
  int i = 0;
  while( source[i] != NULL )
  {
    const char* nameString = (const char*)source[i];
    const char* programString = (const char*)source[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      if( programString == NULL )
        DX9Fail("Pixel shader 2.0 failed to compile.");

      initialize( programString );
      return;
    }

    i += 2;
  }

  DX9Fail("DXKernel failure - no DX9 program string found");
}

void DX9Kernel::PushStream(Stream *s) {
  DX9Stream* stream = (DX9Stream*)s;
  int arg = argumentStreamIndex++;
  inputStreams[arg] = stream;
  PushSampler( stream );
  PushTexCoord( stream->getInputRect() );
}

void DX9Kernel::PushReduce(void * val, __BRTStreamType type) {
  DX9Trace("PushReduce");
  argumentReductionIndex++;
  outputReductionData = val;
  outputReductionType = type;
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
  int arg = argumentConstantIndex++;
  int constantIndex = arg;
  DX9Trace("PushConstant[%d] < %3.2f, %3.2f, %3.2f, %3.2f >",
    constantIndex+kBaseConstantIndex, val.x, val.y, val.z, val.w );
  inputConstants[constantIndex] = val;
}

void DX9Kernel::PushGatherStream(Stream *s) {
  DX9Stream* stream = (DX9Stream*)s;
  PushConstant( stream->getGatherConstant() );
  PushSampler( stream );
}

void DX9Kernel::PushOutput(Stream *s) {
  DX9Trace("PushOutput");
  int arg = argumentOutputIndex++;

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DSurface9* surfaceHandle = stream->getSurfaceHandle();

  outputSurface = surfaceHandle;
  outputRect = stream->getOutputRect();
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

  result = getDevice()->SetRenderTarget( 0, outputSurface );
  DX9CheckResult( result );

//  result = getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, 0xFF0000FF, 1.0f, 0 );
//  DX9CheckResult( result );

  result = getDevice()->BeginScene();
  DX9CheckResult( result );

  result = getDevice()->SetPixelShader( pixelShader->getHandle() );
  DX9CheckResult( result );
  result = getDevice()->SetVertexShader( vertexShader->getHandle() );
  DX9CheckResult( result );
  int i;
  for( i = 0; i < samplerCount; i++ )
  {
    result = getDevice()->SetTexture( i, inputTextures[i] );
    DX9CheckResult( result );
  }

  // TIM: TODO: set up workspace constant

  for( i = 0; i < constantCount; i++ )
  {
    result = getDevice()->SetPixelShaderConstantF( i+kBaseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9CheckResult( result );
  }

  runtime->execute( outputRect, inputRects );

  result = getDevice()->EndScene();
  DX9CheckResult( result );
  result = getDevice()->Present( NULL, NULL, NULL, NULL );
  DX9CheckResult( result );
}

void DX9Kernel::Reduce() {
  DX9Trace("Reduce");

  if( outputReductionType == __BRTSTREAM )
    ReduceToStream();
  else
    ReduceToValue();
}

DX9Kernel::~DX9Kernel() {
  // Does nothing
}

IDirect3DDevice9* DX9Kernel::getDevice() {
  return runtime->getDevice();
}

void DX9Kernel::initialize( const char* source ) {
  pixelShader = DX9PixelShader::create( runtime, source );

  // TIM: initialize all the rects, just in case
  outputRect = DX9Rect(0,0,0,0);
  for( int i = 0; i < 8; i++ )
  {
    inputRects[i] = DX9Rect(0,0,0,0);
    inputTextures[i] = NULL;
  }
}

void DX9Kernel::PushSampler( DX9Stream* s )
{
  int samplerIndex = argumentSamplerIndex++;
  DX9Trace("PushSampler[%d]",samplerIndex);
  IDirect3DTexture9* texture = s->getTextureHandle();
  inputTextures[samplerIndex] = texture;
}

void DX9Kernel::PushTexCoord( const DX9Rect& r )
{
  int textureUnit = argumentTexCoordIndex++;
  DX9Trace("PushTexCoord[%d]",textureUnit);
  inputRects[textureUnit] = r;
}

void DX9Kernel::ClearInputs()
{
  argumentStreamIndex = 0;
  argumentSamplerIndex = 0;
  argumentTexCoordIndex = 0;
  argumentConstantIndex = 0;
  argumentOutputIndex = 0;
  argumentReductionIndex = 0;
}

void DX9Kernel::ReduceToStream()
{
  DX9Fail("Can't reduce to stream right now");
}

static void DumpBuffer( float4* buffer )
{
  int i;
  DX9Print( "Left Buffer:\n" );
  for( i = 0; i < 11; i++ )
  {
    for( int j = 0; j < 11; j++ )
    {
      float4 value = buffer[i*kDX9ReductionBufferWidth + j];
      DX9Print( "{%4.1f %4.1f %4.1f} ", value.x, value.y, value.z );
    }
    DX9Print( "\n" );
  }
  DX9Print( "Right Buffer:\n" );
  for( i = 0; i < 11; i++ )
  {
    for( int j = 0; j < 11; j++ )
    {
      float4 value = buffer[i*kDX9ReductionBufferWidth + (kDX9ReductionBufferWidth/2) + j];
      DX9Print( "{%4.1f %4.1f %4.1f} ", value.x, value.y, value.z );
    }
    DX9Print( "\n" );
  }
}

void DX9Kernel::ReduceToValue()
{
  HRESULT result;

  int streamCount = argumentStreamIndex;
  int constantCount = argumentConstantIndex;
  int outputCount = argumentOutputIndex;
  int reductionCount = argumentReductionIndex;

  // inspect the input stuff:
  if( streamCount != 1 )
    DX9Fail("Only one input sampler allowed on DX9 reduction");

  if( reductionCount != 1 )
    DX9Fail("Must have one and only one reduction argument");

  if( outputCount != 0 )
    DX9Fail("Can't have any other outputs during a reduction");

  DX9VertexShader* passthroughVertexShader = runtime->getPassthroughVertexShader();
  DX9PixelShader* passthroughPixelShader = runtime->getPassthroughPixelShader();

  DX9Stream* inputStream = inputStreams[0];
  int inputWidth = inputStream->getWidth();
  int inputHeight = inputStream->getHeight();

  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  int reductionBufferWidth = reductionBuffer->getWidth();
  int reductionBufferHeight = reductionBuffer->getHeight();
  result = getDevice()->SetRenderTarget( 0, reductionBuffer->getSurfaceHandle() );
  DX9CheckResult( result );

  result = getDevice()->BeginScene();
  DX9CheckResult( result );

  result = getDevice()->SetVertexShader( passthroughVertexShader->getHandle() );
  DX9CheckResult( result );

  // TIM: TODO: set up workspace constant

  // Constants can be set up once and then used for each
  // reduction pass
  for( int i = 0; i < constantCount; i++ )
  {
    result = getDevice()->SetPixelShaderConstantF( i+kBaseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9CheckResult( result );
  }

  // first pass - just copy the data into the reduction buffer...
  // this step *could* be elliminated in the future
  result = getDevice()->SetPixelShader( passthroughPixelShader->getHandle() );
  DX9CheckResult( result );

  inputRects[0] = inputStream->getTextureSubRect( 0, 0, inputWidth, inputHeight );
  outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, inputWidth, inputHeight );
  result = getDevice()->SetTexture( 0, inputStream->getTextureHandle() );
  DX9CheckResult( result );
  runtime->execute( outputRect, inputRects );

  // remaining passes - fold the data in half as needed
  result = getDevice()->SetPixelShader( pixelShader->getHandle() );
  DX9CheckResult( result );

  result = getDevice()->SetTexture( 0, reductionBuffer->getTextureHandle() );
  DX9CheckResult( result );
  result = getDevice()->SetTexture( 1, reductionBuffer->getTextureHandle() );
  DX9CheckResult( result );

//  float4* buffer = new float4[kDX9ReductionBufferWidth*kDX9ReductionBufferHeight];
//  reductionBuffer->getData( (float*)buffer );
//  DumpBuffer( buffer );

  int remainingWidth = inputWidth;
  int remainingHeight = inputHeight;

  int currentSide = 0; // starts out on the left
  int slopWidths[2] = {0,0};
  static const int kSideOffsets[2] = {0,kDX9ReductionBufferWidth/2};
  int coreWidth = inputWidth;

  while( remainingWidth > 1 )
  {
    DX9Trace("core = %d, slopL = %d, slopR = %d, remain = %d",
      coreWidth, slopWidths[0], slopWidths[1], remainingWidth );

    int currentSideWidth = coreWidth + slopWidths[currentSide];
    int widthToMove = currentSideWidth / 2;
    
    slopWidths[currentSide] = currentSideWidth - 2*widthToMove;
    remainingWidth -= widthToMove;
    coreWidth = widthToMove;

    inputRects[0] = reductionBuffer->getInterlacedTextureSubRect(
      kSideOffsets[currentSide] + slopWidths[currentSide], 0,
      kSideOffsets[currentSide] + currentSideWidth, remainingHeight, 2, 1 );
    inputRects[1] = reductionBuffer->getInterlacedTextureSubRect(
      kSideOffsets[currentSide] + slopWidths[currentSide]+1, 0,
      kSideOffsets[currentSide] + currentSideWidth+1, remainingHeight, 2, 1 );
    currentSide = 1 - currentSide;
    outputRect = reductionBuffer->getSurfaceSubRect(
      kSideOffsets[currentSide] + slopWidths[currentSide], 0,
      kSideOffsets[currentSide] + slopWidths[currentSide] + widthToMove, remainingHeight );
    runtime->execute( outputRect, inputRects );
//    reductionBuffer->getData( (float*)buffer );
//    DumpBuffer( buffer );
  }

  int slopHeights[2] = {0,0};
  int coreHeight = inputHeight;

  while( remainingHeight > 1 )
  {
    DX9Trace("core = %d, slopL = %d, slopR = %d, remain = %d",
      coreHeight, slopHeights[0], slopHeights[1], remainingHeight );

    int currentSideHeight = coreHeight + slopHeights[currentSide];
    int heightToMove = currentSideHeight / 2;
    
    slopHeights[currentSide] = currentSideHeight - 2*heightToMove;
    remainingHeight -= heightToMove;
    coreHeight = heightToMove;

    inputRects[0] = reductionBuffer->getInterlacedTextureSubRect(
      kSideOffsets[currentSide], slopHeights[currentSide],
      kSideOffsets[currentSide] + remainingWidth, currentSideHeight, 1, 2 );
    inputRects[1] = reductionBuffer->getInterlacedTextureSubRect(
      kSideOffsets[currentSide], slopHeights[currentSide]+1,
      kSideOffsets[currentSide] + remainingWidth, currentSideHeight+1, 1, 2 );
    currentSide = 1 - currentSide;
    outputRect = reductionBuffer->getSurfaceSubRect(
      kSideOffsets[currentSide], slopHeights[currentSide],
      kSideOffsets[currentSide] + remainingWidth, slopHeights[currentSide] + heightToMove );
    runtime->execute( outputRect, inputRects );
//    reductionBuffer->getData( (float*)buffer );
//    DumpBuffer( buffer );
  }

  result = getDevice()->EndScene();
  DX9CheckResult( result );

  float4 reductionResult = {-1,-1,-1,-1};
  reductionBuffer->getPixelAt( kSideOffsets[currentSide], 0, reductionResult );
  if( outputReductionType == __BRTFLOAT )
    *((float*)outputReductionData) = *((float*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT2 )
    *((float2*)outputReductionData) = *((float2*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT3 )
    *((float3*)outputReductionData) = *((float3*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT4 )
    *((float4*)outputReductionData) = *((float4*)&reductionResult);
  else
    DX9Fail("Invalid reduction target type for DX9");

  ClearInputs();
}

/* TIM: old implementation, works for commutative ops
void DX9Kernel::ReduceToValue()
{
  HRESULT result;

  int streamCount = argumentStreamIndex;
  int constantCount = argumentConstantIndex;
  int outputCount = argumentOutputIndex;
  int reductionCount = argumentReductionIndex;

  // inspect the input stuff:
  if( streamCount != 1 )
    DX9Fail("Only one input sampler allowed on DX9 reduction");

  if( reductionCount != 1 )
    DX9Fail("Must have one and only one reduction argument");

  if( outputCount != 0 )
    DX9Fail("Can't have any other outputs during a reduction");

  DX9VertexShader* passthroughVertexShader = runtime->getPassthroughVertexShader();
  DX9PixelShader* passthroughPixelShader = runtime->getPassthroughPixelShader();

  DX9Stream* inputStream = inputStreams[0];
  int inputWidth = inputStream->getWidth();
  int inputHeight = inputStream->getHeight();

  DX9Texture* reductionBuffer = runtime->getReductionBuffer();
  int reductionBufferWidth = reductionBuffer->getWidth();
  int reductionBufferHeight = reductionBuffer->getHeight();
  result = getDevice()->SetRenderTarget( 0, reductionBuffer->getSurfaceHandle() );
  DX9CheckResult( result );

  result = getDevice()->BeginScene();
  DX9CheckResult( result );

  result = getDevice()->SetVertexShader( passthroughVertexShader->getHandle() );
  DX9CheckResult( result );

  // TIM: TODO: set up workspace constant

  // Constants can be set up once and then used for each
  // reduction pass
  for( int i = 0; i < constantCount; i++ )
  {
    result = getDevice()->SetPixelShaderConstantF( i+kBaseConstantIndex, (float*)&(inputConstants[i]), 1 );
    DX9CheckResult( result );
  }

  // first pass - just copy the data into the reduction buffer...
  // this step *could* be elliminated in the future
  result = getDevice()->SetPixelShader( passthroughPixelShader->getHandle() );
  DX9CheckResult( result );

  inputRects[0] = inputStream->getTextureSubRect( 0, 0, inputWidth, inputHeight );
  outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, inputWidth, inputHeight );
  result = getDevice()->SetTexture( 0, inputStream->getTextureHandle() );
  DX9CheckResult( result );
  runtime->execute( outputRect, inputRects );

  // remaining passes - fold the data in half as needed
  result = getDevice()->SetPixelShader( pixelShader->getHandle() );
  DX9CheckResult( result );

  result = getDevice()->SetTexture( 0, reductionBuffer->getTextureHandle() );
  DX9CheckResult( result );
  result = getDevice()->SetTexture( 1, reductionBuffer->getTextureHandle() );
  DX9CheckResult( result );
  int remainingWidth = inputWidth;
  int remainingHeight = inputHeight;

  while( remainingWidth > 1 )
  {
    int columnsToRemove = remainingWidth/2;
    inputRects[0] = reductionBuffer->getTextureSubRect( 0, 0, columnsToRemove, remainingHeight );
    inputRects[1] = reductionBuffer->getTextureSubRect( remainingWidth-columnsToRemove, 0, remainingWidth, remainingHeight );
    outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, columnsToRemove, remainingHeight );
    runtime->execute( outputRect, inputRects );
    remainingWidth -= columnsToRemove;
  }
  while( remainingHeight > 1 )
  {
    int rowsToRemove = remainingHeight/2;
    inputRects[0] = reductionBuffer->getTextureSubRect( 0, 0, remainingWidth, rowsToRemove );
    inputRects[1] = reductionBuffer->getTextureSubRect( 0, remainingHeight-rowsToRemove, remainingWidth, remainingHeight );
    outputRect = reductionBuffer->getSurfaceSubRect( 0, 0, remainingWidth, rowsToRemove );
    runtime->execute( outputRect, inputRects );
    remainingHeight -= rowsToRemove;
   } 

  result = getDevice()->EndScene();
  DX9CheckResult( result );

  float4 reductionResult = {-1,-1,-1,-1};
  reductionBuffer->getTopLeftPixel( reductionResult );
  if( outputReductionType == __BRTFLOAT )
    *((float*)outputReductionData) = *((float*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT2 )
    *((float2*)outputReductionData) = *((float2*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT3 )
    *((float3*)outputReductionData) = *((float3*)&reductionResult);
  else if( outputReductionType == __BRTFLOAT4 )
    *((float4*)outputReductionData) = *((float4*)&reductionResult);
  else
    DX9Fail("Invalid reduction target type for DX9");

  ClearInputs();
}
*/
