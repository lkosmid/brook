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

DX9RunTime::DX9RunTime()
  : reductionBuffer(NULL)
{
  // XXX: TO DO
  // TIM: initialize D3D
  DX9Trace("DX9RunTime::DX9RunTime");

  vertexBuffer = NULL;
  indexBuffer = NULL;
  vertexDecl = NULL;

  window = DX9Window::create();
  HWND windowHandle = window->getWindowHandle();

	direct3D = Direct3DCreate9( D3D_SDK_VERSION );
	if( direct3D == NULL )
		throw 1; // TIM: TODO: better errors

	D3DPRESENT_PARAMETERS deviceDesc;
	ZeroMemory( &deviceDesc, sizeof(deviceDesc) );

	deviceDesc.Windowed = TRUE;
	deviceDesc.SwapEffect = D3DSWAPEFFECT_DISCARD;
	deviceDesc.BackBufferFormat = D3DFMT_UNKNOWN;
	deviceDesc.EnableAutoDepthStencil = FALSE;
	deviceDesc.AutoDepthStencilFormat = D3DFMT_D24S8;

	HRESULT result = direct3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &deviceDesc, &device );
	DX9CheckResult( result );

	// TIM: set up initial state
	device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
//	device->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );

  passthroughVertexShader = DX9VertexShader::create( this, kPassthroughVertexShaderSource );
  passthroughPixelShader = DX9PixelShader::create( this, kPassthroughPixelShaderSource );
}

Kernel * DX9RunTime::CreateKernel(const void* source[]) {
  return new DX9Kernel( this, source );
}

Stream * DX9RunTime::CreateStream(__BRTStreamType type, int dims, int extents[]) {
  // XXX: TO DO
  return new DX9Stream( this, type, dims, extents );
}

DX9RunTime::~DX9RunTime() {
  // Does nothing
  // TIM: finalize D3D
}

void DX9RunTime::execute( const DX9Rect& outputRect, const DX9Rect* inputRects )
{
  HRESULT result;
  initializeVertexBuffer();

  DX9Vertex* vertices;
  result = vertexBuffer->Lock( 0, 0, (void**)&vertices, D3DLOCK_DISCARD );
  DX9CheckResult( result );

  DX9Trace("execute:");
  DX9Trace("position: (%f, %f) - (%f, %f)",
    outputRect.left, outputRect.top, outputRect.right, outputRect.bottom );
  DX9Trace("texture0: (%f, %f) - (%f, %f)",
    inputRects[0].left, inputRects[0].top, inputRects[0].right, inputRects[0].bottom );
  DX9Trace("texture1: (%f, %f) - (%f, %f)",
    inputRects[1].left, inputRects[1].top, inputRects[1].right, inputRects[1].bottom );

  DX9Vertex vertex;
  for( int i = 0; i < 4; i++ )
  {
    int xIndex = (i&0x01) ? 0 : 2;
    int yIndex = (i&0x02) ? 3 : 1;

    // TIM: bad
    vertex.position.x = outputRect[xIndex];
    vertex.position.y = outputRect[yIndex];
    vertex.position.z = 0.5f;
    vertex.position.w = 1.0f;

    for( int t = 0; t < 8; t++ )
    {
      vertex.texcoords[t].x = inputRects[t][xIndex];
      vertex.texcoords[t].y = inputRects[t][yIndex];
    }

    *vertices++ = vertex;
  }
  result = vertexBuffer->Unlock();
  DX9CheckResult( result );

  result = device->SetVertexDeclaration( vertexDecl );
  DX9CheckResult( result );

  result = device->SetStreamSource( 0, vertexBuffer, 0, sizeof(DX9Vertex) );
  DX9CheckResult( result );

  result = device->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  DX9CheckResult( result );
}

DX9Texture* DX9RunTime::getReductionBuffer() {
  if( reductionBuffer != NULL ) return reductionBuffer;

  reductionBuffer = DX9Texture::create( this, kDX9ReductionBufferWidth, kDX9ReductionBufferHeight, 4 );
  return reductionBuffer;
}

static const D3DVERTEXELEMENT9 kDX9VertexElements[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 4*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 6*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 8*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
	{ 0, 10*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
	{ 0, 12*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },
	{ 0, 14*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5 },
	{ 0, 16*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 6 },
	{ 0, 18*sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 7 },
	D3DDECL_END()
};

void DX9RunTime::initializeVertexBuffer()
{
  if( vertexBuffer != NULL ) return;

  static const int kMaxVertexCount = 64;
  static const int kMaxIndexCount = 128;

  HRESULT result = device->CreateVertexBuffer(
    kMaxVertexCount*sizeof(DX9Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL );
  DX9CheckResult( result );

  result = device->CreateIndexBuffer(
    kMaxIndexCount*sizeof(UInt16), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &indexBuffer, NULL );
  DX9CheckResult( result );

  result = device->CreateVertexDeclaration( kDX9VertexElements, &vertexDecl );
  DX9CheckResult( result );
}
