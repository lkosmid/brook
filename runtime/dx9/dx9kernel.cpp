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

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = mapArgumentToTextureUnit( arg );
  getDevice()->SetTexture( textureUnit, textureHandle );

  // TIM: TODO:
  // setup rectangle to be rendered from...
}

void DX9Kernel::SetConstantFloat(const int arg, const float &val) {
  DX9Trace("SetConstantFloat");

  float4 value;
  value.x = val;
  value.y = 0;
  value.z = 0;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  getDevice()->SetPixelShaderConstantF( constantIndex, (float*)&value, 1 );
}

void DX9Kernel::SetConstantFloat2(const int arg, const float2 &val) {
  DX9Trace("SetConstantFloat2");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = 0;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  getDevice()->SetPixelShaderConstantF( constantIndex, (float*)&value, 1 );
}

void DX9Kernel::SetConstantFloat3(const int arg, const float3 &val) {
  DX9Trace("SetConstantFloat3");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = val.z;
  value.w = 1;

  int constantIndex = mapArgumentToConstantIndex( arg );
  getDevice()->SetPixelShaderConstantF( constantIndex, (float*)&value, 1 );
}

void DX9Kernel::SetConstantFloat4(const int arg, const float4 &val) {
  DX9Trace("SetConstantFloat4");

  float4 value;
  value.x = val.x;
  value.y = val.y;
  value.z = val.z;
  value.w = val.w;

  int constantIndex = mapArgumentToConstantIndex( arg );
  getDevice()->SetPixelShaderConstantF( constantIndex, (float*)&value, 1 );
}

void DX9Kernel::SetGatherInput(const int arg, const __BrookStream *s) {
  DX9Trace("SetGatherInput");
  
  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DTexture9* textureHandle = stream->getTextureHandle();

  int textureUnit = mapArgumentToTextureUnit( arg );
  getDevice()->SetTexture( textureUnit, textureHandle );
}

void DX9Kernel::SetOutput(const __BrookStream *s) {
  DX9Trace("SetOutput");

  DX9Stream* stream = (DX9Stream*)s;
  IDirect3DSurface9* surfaceHandle = stream->getSurfaceHandle();

  getDevice()->SetRenderTarget( 0, surfaceHandle );

  // TIM: TODO:
  // set up the vertex rect to be rendered to...
}

void DX9Kernel::Exec(void) {
  DX9Trace("Exec");

  DX9VertexShader* vertexShader = runtime->getPassthroughVertexShader();

  getDevice()->SetPixelShader( pixelShader->getHandle() );
  getDevice()->SetVertexShader( vertexShader->getHandle() );

  // XXX: TODO
  // render with pre-determined rects...
}

DX9Kernel::~DX9Kernel() {
  // Does nothing
}

IDirect3DDevice9* DX9Kernel::getDevice() {
  return runtime->getDevice();
}

void DX9Kernel::initialize( const char* source ) {
  pixelShader = DX9PixelShader::create( runtime, source );
}

int DX9Kernel::mapArgumentToTextureUnit( int arg ) {
  // TIM: totally hacked for now
  return arg;
}

int DX9Kernel::mapArgumentToConstantIndex( int arg ) {
  // TIM: totally hacked for now
  return arg;
}