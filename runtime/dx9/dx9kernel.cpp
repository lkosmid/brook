#include "dx9.hpp"

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"

using namespace brook;

static const char* PIXEL_SHADER_NAME_STRING = "ps20";

DX9Kernel::DX9Kernel(DX9RunTime* runtime, const void* source[])
  : runtime(runtime), argumentStreamIndex(0), argumentConstantIndex(0), argumentOutputIndex(0)
{
  DX9Trace("DX9Kernel::DX9Kernel");
  
  int i = 0;
  while( source[i] != NULL )
  {
    const char* nameString = (const char*)source[i];
    const char* programString = (const char*)source[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      initialize( programString );
      return;
    }

    i += 2;
  }

  DX9Fail("DXKernel failure - no DX9 program string found");
}

void DX9Kernel::PushStream(Stream *s) {
  DX9Trace("PushInput");
  int arg = argumentStreamIndex++;

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = arg;
  inputTextures[textureUnit] = textureHandle;
  inputRects[textureUnit] = stream->getInputRect();
  DX9Trace("PushInput - end");
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
  DX9Trace("PushConstant");
  int arg = argumentConstantIndex++;
  int constantIndex = arg;
  inputConstants[constantIndex] = val;
}

void DX9Kernel::PushGatherStream(Stream *s) {
  DX9Trace("PushGatherStream");
  int arg = argumentStreamIndex++;
  
  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = arg;
  inputTextures[textureUnit] = textureHandle;
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

  int streamCount = argumentStreamIndex;
  int constantCount = argumentConstantIndex;
  int outputCount = argumentOutputIndex;
  argumentStreamIndex = 0;
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
  for( i = 0; i < streamCount; i++ )
  {
    result = getDevice()->SetTexture( i, inputTextures[i] );
    DX9CheckResult( result );
  }

  for( i = 0; i < constantCount; i++ )
  {
    result = getDevice()->SetPixelShaderConstantF( i, (float*)&(inputConstants[i]), 1 );
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
