#include <dx9.hpp>

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

  // TODO: handle error - no such program string
  throw 1;
}

void DX9Kernel::SetInput(const int arg, const __BrookStream *s) {
  DX9Trace("SetInput");
  // XXX: TODO
  // decodes texture and range of coords (for subtexture)
  // from stream and stores for execute stage
  // getDevice()->SetTexture( arg, s->getTextureHandle() );
  // textureRect[arg] = s->getTextureRect();
}

void DX9Kernel::SetConstantFloat(const int arg, const float &val) {
  DX9Trace("SetConstantFloat");
  // XXX: TODO
  // directly set PS constant
  // float4 value;
  // value.x = val;
  // value.y = 0;
  // value.z = 0;
  // value.w = 1;
  // getDevice()->SetPixelShaderConstantF( arg, &value, 1 );
}

void DX9Kernel::SetConstantFloat2(const int arg, const float2 &val) {
  DX9Trace("SetConstantFloat2");
  // XXX: TODO
}

void DX9Kernel::SetConstantFloat3(const int arg, const float3 &val) {
  DX9Trace("SetConstantFloat3");
  // XXX: TODO
}

void DX9Kernel::SetConstantFloat4(const int arg, const float4 &val) {
  DX9Trace("SetConstantFloat4");
  // XXX: TODO
}

void DX9Kernel::SetGatherInput(const int arg, const __BrookStream *s) {
  DX9Trace("SetGatherInput");
  // XXX: TODO
  // sets up texture for read, but doesn't worry about setting up
  // texcoords for it...
  // getDevice()->SetTexture( arg, s->getTextureHandle() );
}

void DX9Kernel::SetOutput(const __BrookStream *s) {
  DX9Trace("SetOutput");
  // XXX: TODO
  // sets render target, and decodes destination vertex rect
  // getDevice()->SetRenderTarget( 0, s->getRenderTargetHandle() );
  // vertexRect = s->getVertexRect();
}

void DX9Kernel::Exec(void) {
  DX9Trace("Exec");
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
  DX9Trace("initialize"); 
}