// renderWindow.cpp
#include "renderWindow.hpp"

#include "built/simulationKernel.hpp"

static const int kWindowSize = 512;
static const int kFluidSize = 256;

static const char* kPassthroughVertexShader =
"vs.1.1\n"
"dcl_position v0\n"
"dcl_texcoord0 v1\n"
"mov oPos, v0\n"
"mov oT0, v1\n"
;

static const char* kPassthroughPixelShader =
"ps_2_0\n"
"dcl t0.xy\n"
"dcl_2d s0\n"
"texld r0, t0, s0\n"

"mov r1, c4\n"

//"dp3 r0.w, r0, c0\n" // diffuse
//"mad r1, r0.w, c1, r1\n"

"dp3 r0.w, r0, c2\n" // specular
"mul r0.w, r0.w, r0.w\n" // specexp = 2
"mul r0.w, r0.w, r0.w\n" // specexp = 4
"mul r0.w, r0.w, r0.w\n" // specexp = 8
"mad r1, r0.w, c3, r1\n"

"mov oC0, r1\n"
;

static const D3DVERTEXELEMENT9 kVertexFormat[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 4*sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

struct Vertex
{
  float x, y, z, w;
  float tx, ty, tz, tw;
};


RenderWindow::RenderWindow()
  : Window( kWindowSize, kWindowSize ),
  context(NULL),
  device(NULL),
  mouseDown(false)
{
  using namespace fibble; // stupid DX9 wrapper classes
  using namespace brook; // Brook Runtime classes
  
  context = Context::create( this );
  device = context->getDevice();

  vertexShader = context->createVertexShader( kPassthroughVertexShader );
  pixelShader = context->createPixelShader( kPassthroughPixelShader );

  HRESULT result = device->CreateVertexBuffer(
    4*sizeof(Vertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL );
  if( FAILED(result) ) throw -1;

  result = device->CreateVertexDeclaration( kVertexFormat, &vertexDecl );
  if( FAILED(result) ) throw -1;

  result = device->GetRenderTarget( 0, &defaultRenderTarget );
  if( FAILED(result) ) throw -1;

  // Initialize the Brook Runtime
  // We pass in the identifier of the runtime we wish to use,
  // as well as a context value that holds our existing
  // rendering device...
  brook::initialize( "dx9", (void*)device );

  // Create streams
  fluidStream0 = stream::create<float4>( kFluidSize, kFluidSize );
  fluidStream1 = stream::create<float4>( kFluidSize, kFluidSize );
  normalStream = stream::create<float3>( kFluidSize, kFluidSize );

  // Get a handle to the texture being used by the normal stream as a backing store
  normalTexture = (IDirect3DTexture9*)(normalStream->getIndexedFieldRenderData(0));

  // Initialize the fluid data
  clearKernel( fluidStream0 );
}

RenderWindow::~RenderWindow()
{
}

static void makeNormalConstant( float x, float y, float z, float4& outResult )
{
  float invLength = 1.0f / sqrtf( x*x + y*y + z*z );
  outResult.x = x * invLength;
  outResult.y = y * invLength;
  outResult.z = z * invLength;
  outResult.w = 1;
}

static void makeColorConstant( float r, float g, float b, float4& outResult )
{
  outResult.x = r;
  outResult.y = g;
  outResult.z = b;
  outResult.w = 1;
}


void RenderWindow::handleIdle()
{
  HRESULT result;

  // Call kernels to execute the simulation
  
  // The simulation kernel integrates the heightfield under
  // neighbor forces interactions (diffusion)
  float4 controlConstant(0,0,0,0);
  if( mouseDown )
  {
    float controlRadius = 8.0f;
    float controlHeight = -5.0f;
    controlConstant = float4( (float)mouseX, (float)mouseY, controlHeight, controlRadius*controlRadius );
  }
  simulationKernel( fluidStream0, fluidStream0, controlConstant, fluidStream1 );

  // The smoothing kernel does a very simple neighbor-sampling filter
  // to help keep the simulation stable and smooth
  smoothKernel( fluidStream1, fluidStream1, fluidStream0 );

  // The normal generation kernel (not surprisingly) generates a normal
  // map from the heightfield, allowing us to draw it with bump-mapping
  normalGenerationKernel( fluidStream0, normalStream );

  // We call "synchronizeRenderData" on the normal stream to make sure
  // that it's data is available in a texture, and then bind it
  normalStream->synchronizeRenderData();
  result = device->SetTexture( 0, normalTexture );
  if( FAILED(result) ) throw -1;

  result = device->SetRenderTarget( 0, defaultRenderTarget );
  if( FAILED(result) ) throw -1;

  // render
  if( !context->beginScene() ) return;
  context->clear();

  context->setVertexShader( vertexShader );
  context->setPixelShader( pixelShader );

  float4 pixelShaderConst;
  makeNormalConstant( 1, -1, 2, pixelShaderConst );
  device->SetPixelShaderConstantF( 0, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.1f, 0.1f, 0.5f, pixelShaderConst );
  device->SetPixelShaderConstantF( 1, (float*)&pixelShaderConst, 1 );
  makeNormalConstant( 1, -1, 2, pixelShaderConst );
  device->SetPixelShaderConstantF( 2, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.8f, 0.8f, 1.0, pixelShaderConst );
  device->SetPixelShaderConstantF( 3, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.1f, 0.1f, 0.5f, pixelShaderConst );
  device->SetPixelShaderConstantF( 4, (float*)&pixelShaderConst, 1 );

  Vertex* vertices;
  result = vertexBuffer->Lock( 0, 0, (void**)&vertices, D3DLOCK_DISCARD );
  if( FAILED(result) ) throw -1;

  Vertex vertex;

  float left = -1.0f, right = 1.0f, top = 1.0f, bottom = -1.0f;
  float texleft = 0.0f, texright = 1.0f, textop = 0.0f, texbottom = 1.0f;

  for( int i = 0; i < 4; i++ )
  {
    vertex.x = (i & 1) ? right : left;
    vertex.y = (i & 2) ? bottom : top;
    vertex.z = 0.5f;
    vertex.w = 1.0f;

    vertex.tx = (i & 1) ? texright : texleft;
    vertex.ty = (i & 2) ? texbottom : textop;
    vertex.tz = 0.5f;
    vertex.tw = 1.0f;

    *vertices++ = vertex;
  }
  result = vertexBuffer->Unlock();
  if( FAILED(result) ) throw -1;

  result = device->SetVertexDeclaration( vertexDecl );
  if( FAILED(result) ) throw -1;

  result = device->SetStreamSource( 0, vertexBuffer, 0, sizeof(Vertex) );
  if( FAILED(result) ) throw -1;

  result = device->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  if( FAILED(result) ) throw -1;

  context->endScene();
  context->swap();
}

void RenderWindow::handleMouseDown( int x, int y )
{
  mouseDown = true;
}

void RenderWindow::handleMouseUp( int x, int y )
{
  mouseDown = false;
}

void RenderWindow::handleMouseMove( int x, int y )
{
  mouseX = (float)kFluidSize * (float)x / (float)kWindowSize;
  mouseY = (float)kFluidSize * (float)y / (float)kWindowSize;
}