#include <dx9.hpp>

#include "dx9pixelshader.hpp"
#include "dx9vertexshader.hpp"

static const char* PIXEL_SHADER_NAME_STRING = "ps2.0";

DX9Kernel::DX9Kernel(DX9RunTime* runtime, const char* source[])
  : runtime(runtime)
{
  DX9Trace("DX9Kernel::DX9Kernel");
  
  int i = 0;
  while( source[i] != NULL )
  {
    const char* nameString = source[i];
    const char* programString = source[i+1];

    if( strncmp( nameString, PIXEL_SHADER_NAME_STRING, strlen(PIXEL_SHADER_NAME_STRING) ) == 0 )
    {
      initialize( programString );
      return;
    }

    i += 2;
  }

  DX9Fail("DXKernel failure - no DX9 program string found");
}

void DX9Kernel::SetInput(const int arg, const __BrookStream *s) {
  DX9Trace("SetInput");
  HRESULT result;

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = mapArgumentToTextureUnit( arg );
  inputTextures[textureUnit] = textureHandle;
  inputRects[textureUnit] = stream->getInputRect();
}

void DX9Kernel::SetConstantFloat(const int arg, const float &val) {
  DX9Trace("SetConstantFloat");

  float4 value;
  value.x = val;
  value.y = 0;
  value.z = 0;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  inputConstants[constantIndex] = value;
}

void DX9Kernel::SetConstantFloat2(const int arg, const float2 &val) {
  DX9Trace("SetConstantFloat2");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = 0;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  inputConstants[constantIndex] = value;
}

void DX9Kernel::SetConstantFloat3(const int arg, const float3 &val) {
  DX9Trace("SetConstantFloat3");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = val.z;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  inputConstants[constantIndex] = value;
}

void DX9Kernel::SetConstantFloat4(const int arg, const float4 &val) {
  DX9Trace("SetConstantFloat4");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = val.z;
  value.w = val.w;

  int constantIndex = mapArgumentToConstantIndex( arg );
  inputConstants[constantIndex] = value;
}

void DX9Kernel::SetGatherInput(const int arg, const __BrookStream *s) {
  DX9Trace("SetGatherInput");
  
  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = mapArgumentToTextureUnit( arg );
  inputTextures[textureUnit] = textureHandle;
}

void DX9Kernel::SetOutput(const __BrookStream *s) {
  DX9Trace("SetOutput");

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DSurface9* surfaceHandle = stream->getSurfaceHandle();

  outputSurface = surfaceHandle;
  outputRect = stream->getOutputRect();
}

void DX9Kernel::Exec(void) {
  DX9Trace("Exec");
  HRESULT result;

  DX9VertexShader* vertexShader = runtime->getPassthroughVertexShader();

  result = getDevice()->SetRenderTarget( 0, outputSurface );
  DX9CheckResult( result );

  result = getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, 0xFF0000FF, 1.0f, 0 );
  DX9CheckResult( result );

  result = getDevice()->BeginScene();
  DX9CheckResult( result );

  result = getDevice()->SetPixelShader( pixelShader->getHandle() );
  DX9CheckResult( result );
  result = getDevice()->SetVertexShader( vertexShader->getHandle() );
  DX9CheckResult( result );

  for( int i = 0; i < 8; i++ )
  {
    result = getDevice()->SetTexture( i, inputTextures[i] );
    DX9CheckResult( result );
  }

  for( int i = 0; i < 8; i++ )
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

int DX9Kernel::mapArgumentToTextureUnit( int arg ) {
  // TIM: totally hacked for now
  return arg-1;
}

int DX9Kernel::mapArgumentToConstantIndex( int arg ) {
  // TIM: totally hacked for now
  return arg-1;
}