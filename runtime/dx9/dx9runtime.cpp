#include "dx9.hpp"
#include "dx9base.hpp"

#include "dx9window.hpp"
#include "dx9vertexshader.hpp"
#include "dx9pixelshader.hpp"
#include "dx9texture.hpp"

using namespace brook;

namespace brook {
  const char* DX9_RUNTIME_STRING = "dx9";
}

static const char* kPassthroughVertexShaderSource =
"vs.1.1\n"
"dcl_position v0\n"
"dcl_texcoord0 v1\n"
"dcl_texcoord1 v2\n"
"dcl_texcoord2 v3\n"
"dcl_texcoord3 v4\n"
"dcl_texcoord4 v5\n"
"dcl_texcoord5 v6\n"
"dcl_texcoord6 v7\n"
"dcl_texcoord7 v8\n"
"mov oPos, v0\n"
"mov oT0, v1\n"
"mov oT1, v2\n"
"mov oT2, v3\n"
"mov oT3, v4\n"
"mov oT4, v5\n"
"mov oT5, v6\n"
"mov oT6, v7\n"
"mov oT7, v8\n"
;

static const char* kPassthroughPixelShaderSource =
"ps_2_0\n"
"dcl t0.xy\n"
"dcl_2d s0\n"
"texld r0, t0, s0\n"
"mov oC0, r0\n"
;

DX9RunTime* DX9RunTime::create()
{
  DX9RunTime* result = new DX9RunTime();
  if( result->initialize() )
    return result;
  delete result;
  return NULL;
}

DX9RunTime::DX9RunTime()
  : window(NULL),
  passthroughVertexShader(NULL),
  passthroughPixelShader(NULL),
  reductionBuffer(NULL),
  reductionTargetBuffer(NULL),
  direct3D(NULL),
  device(NULL),
  vertexBuffer(NULL),
  vertexDecl(NULL)
{
}

bool DX9RunTime::initialize()
{
  window = DX9Window::create();
  if( window == NULL )
  {
    DX9Warn("Could not create offscreen window.");
    return false;
  }
  HWND windowHandle = window->getWindowHandle();

	direct3D = Direct3DCreate9( D3D_SDK_VERSION );
  if( direct3D == NULL )
  {
    DX9Warn("Could not create Direct3D interface.");
    return false;
  }

	D3DPRESENT_PARAMETERS deviceDesc;
	ZeroMemory( &deviceDesc, sizeof(deviceDesc) );

	deviceDesc.Windowed = TRUE;
	deviceDesc.SwapEffect = D3DSWAPEFFECT_DISCARD;
	deviceDesc.BackBufferFormat = D3DFMT_UNKNOWN;
	deviceDesc.EnableAutoDepthStencil = FALSE;
	deviceDesc.AutoDepthStencilFormat = D3DFMT_D24S8;

	HRESULT result = direct3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &deviceDesc, &device );
  if( FAILED(result) )
  {
    DX9Warn("Could not create Direct3D device.");
    return false;
  }

	// TIM: set up initial state
	result = device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
  DX9AssertResult( result, "SetRenderState failed" );

  passthroughVertexShader = DX9VertexShader::create( this, kPassthroughVertexShaderSource );
  if( passthroughVertexShader == NULL )
  {
    DX9Warn("Could not create passthrough vertex shader.");
    return false;
  }

  passthroughPixelShader = DX9PixelShader::create( this, kPassthroughPixelShaderSource );
  if( passthroughPixelShader == NULL )
  {
    DX9Warn("Could not create passthrough pixel shader.");
    return false;
  }

  return true;
}

DX9RunTime::~DX9RunTime()
{
  if( vertexDecl != NULL )
    vertexDecl->Release();
  if( vertexBuffer != NULL )
    vertexBuffer->Release();
  if( reductionBuffer != NULL )
    delete reductionBuffer;
  if( reductionTargetBuffer != NULL )
    delete reductionTargetBuffer;
  if( passthroughPixelShader != NULL )
    delete passthroughPixelShader;
  if( passthroughVertexShader != NULL )
    delete passthroughVertexShader;
  if( device != NULL )
    device->Release();
  if( direct3D != NULL )
    direct3D->Release();
  if( window != NULL )
    delete window;
}

Kernel* DX9RunTime::CreateKernel(const void* source[]) {
  Kernel* result = DX9Kernel::create( this, source );
  DX9Assert( result != NULL, "Unable to allocate a kernel, exiting." );
  return result;
}

Stream* DX9RunTime::CreateStream(__BRTStreamType type, int dims, int extents[])
{
  Stream* result = DX9Stream::create( this, type, dims, extents );
  DX9Assert( result != NULL, "Unable to allocate a stream, exiting." );
  return result;
}

Iter* DX9RunTime::CreateIter(
  __BRTStreamType type, int dims, int extents[], float r[] )
{
  Iter* result = DX9Iter::create( this, type, dims, extents , r );
  DX9Assert( result != NULL, "Unable to allocate an iterator, exiting." );
  return result;
}

void DX9RunTime::execute( const DX9FatRect& outputRect, const DX9FatRect* inputRects )
{
  HRESULT result;
  initializeVertexBuffer();

  DX9Vertex* vertices;
  result = vertexBuffer->Lock( 0, 0, (void**)&vertices, D3DLOCK_DISCARD );
  DX9AssertResult( result, "VB::Lock failed" );

  DX9Vertex vertex;
  for( int i = 0; i < 4; i++ )
  {
    float4 position = outputRect.vertices[i];

    // TIM: bad
    vertex.position.x = position.x;
    vertex.position.y = position.y;
    vertex.position.z = 0.5f;
    vertex.position.w = 1.0f;

    for( int t = 0; t < 8; t++ )
      vertex.texcoords[t] = inputRects[t].vertices[i];

    *vertices++ = vertex;
  }
  result = vertexBuffer->Unlock();
  DX9AssertResult( result, "VB::Unlock failed" );

  result = device->SetVertexDeclaration( vertexDecl );
  DX9AssertResult( result, "SetVertexDeclaration failed" );

  result = device->SetStreamSource( 0, vertexBuffer, 0, sizeof(DX9Vertex) );
  DX9AssertResult( result, "SetStreamSource failed" );

  result = device->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  DX9AssertResult( result, "DrawPrimitive failed" );
}

DX9Texture* DX9RunTime::getReductionBuffer() {
  if( reductionBuffer != NULL ) return reductionBuffer;

  reductionBuffer = DX9Texture::create( this, kDX9ReductionBufferWidth, kDX9ReductionBufferHeight, 4 );
  DX9Assert( reductionBuffer != NULL, "Failed to allocate reduction buffer." );
  return reductionBuffer;
}

DX9Texture* DX9RunTime::getReductionTargetBuffer() {
  if( reductionTargetBuffer != NULL ) return reductionTargetBuffer;

  reductionTargetBuffer = DX9Texture::create( this, 1, 1, 4 );
  DX9Assert( reductionTargetBuffer != NULL, "Failed to allocate reduction target buffer." );
  return reductionTargetBuffer;
}

static const D3DVERTEXELEMENT9 kDX9VertexElements[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 4*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 8*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 12*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
	{ 0, 16*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
	{ 0, 20*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
	{ 0, 24*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 },
	{ 0, 28*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6 },
	{ 0, 32*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7 },
	D3DDECL_END()
};

void DX9RunTime::initializeVertexBuffer()
{
  if( vertexBuffer != NULL ) return;

  static const int kMaxVertexCount = 64;

  HRESULT result = device->CreateVertexBuffer(
    kMaxVertexCount*sizeof(DX9Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL );
  DX9AssertResult( result, "CreateVertexBuffer failed" );

  result = device->CreateVertexDeclaration( kDX9VertexElements, &vertexDecl );
  DX9AssertResult( result, "CreateVertexDeclaration failed" );
}
