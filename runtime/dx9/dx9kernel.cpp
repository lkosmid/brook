#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"

using namespace brook;

static const char* PIXEL_SHADER_NAME_STRING = "ps20";

DX9Kernel::DX9Kernel(DX9RunTime* runtime, const void* source[])
  : runtime(runtime), argumentSamplerIndex(0), argumentTexCoordIndex(0), argumentConstantIndex(0), argumentOutputIndex(0)
{
  DX9Trace("DX9Kernel::DX9Kernel");
  
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
  PushSampler( stream );
  PushTexCoord( stream->getInputRect() );
}

void DX9Kernel::PushReduce(void * val, __BRTStreamType type) {
   //XXX Add Reduce functionality
   fprintf (stderr,"Reduce inoperative in DirectX\n");
   switch (type) {
   case __BRTFLOAT2:
      PushConstant(*(float2*)val);
      break;
   case __BRTFLOAT3:
      PushConstant(*(float3*)val);
      break;
   case __BRTFLOAT4:
      PushConstant(*(float4*)val);
      break;
   default:
      PushConstant(*(float*)val);
   }  
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
  argumentSamplerIndex = 0;
  argumentTexCoordIndex = 0;
  argumentConstantIndex = 0;
  argumentOutputIndex = 0;

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
