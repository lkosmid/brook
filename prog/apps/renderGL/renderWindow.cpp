// renderWindow.cpp
#include "renderWindow.hpp"

#include "built/simulationKernel.hpp"

static const int kWindowSize = 1024; //512;
static const int kFluidSize = 512; //256;

static const char* kPassthroughPixelShader =
   "!!ARBfp1.0\n"
   "TEX result.color, fragment.texcoord[0], texture[0], RECT;\n"
   "END\n";

struct Vertex
{
  float x, y, z, w;
  float tx, ty, tz, tw;
};


RenderWindow::RenderWindow()
  : Window( kWindowSize, kWindowSize ),
  context(NULL),
  mouseDown(false)
{
  using namespace fibble; // stupid DX9 wrapper classes
  using namespace brook; // Brook Runtime classes
  
  context = Context::create( this );

  // Initialize the Brook Runtime
  // We pass in the identifier of the runtime we wish to use,
  // as well as a context value that holds our existing
  // rendering device...
  brook::initialize( "ogl", (void*) context->getGLContext() );
  brook::unbind();

  /*
   * These must be created after brook::initialize() or else there can be
   * weird collisions in the program IDs they're given with the ones BRT
   * uses internally.
   */

  context->bind();
  pixelShader = context->createPixelShader( kPassthroughPixelShader );


  // Create streams
  brook::bind();
  fluidStream0 = stream::create<float4>( kFluidSize, kFluidSize );
  fluidStream1 = stream::create<float4>( kFluidSize, kFluidSize );
  normalStream = stream::create<float3>( kFluidSize, kFluidSize );

  // Get a handle to the texture being used by the normal stream as a backing store
  normalTexture = (GLuint) (normalStream->getIndexedFieldRenderData(0));

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
//  HRESULT result;

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
  brook::bind();

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
//  result = device->SetTexture( 0, normalTexture );
//  if( FAILED(result) ) throw -1;

//  result = device->SetRenderTarget( 0, defaultRenderTarget );
//  if( FAILED(result) ) throw -1;

  // render

  brook::unbind();
  context->bind();

  if( !context->beginScene() ) return;
  context->clear();

  context->setPixelShader( pixelShader );

  float4 pixelShaderConst;
  makeNormalConstant( 1, -1, 2, pixelShaderConst );
//  device->SetPixelShaderConstantF( 0, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.1f, 0.1f, 0.5f, pixelShaderConst );
//  device->SetPixelShaderConstantF( 1, (float*)&pixelShaderConst, 1 );
  makeNormalConstant( 1, -1, 2, pixelShaderConst );
//  device->SetPixelShaderConstantF( 2, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.8f, 0.8f, 1.0, pixelShaderConst );
//  device->SetPixelShaderConstantF( 3, (float*)&pixelShaderConst, 1 );
  makeColorConstant( 0.1f, 0.1f, 0.5f, pixelShaderConst );
//  device->SetPixelShaderConstantF( 4, (float*)&pixelShaderConst, 1 );

  Vertex vertex;

  float left = -1.0f, right = 1.0f, top = 1.0f, bottom = -1.0f;
  float texleft = 0.0f, texright = kFluidSize, textop = 0.0f, texbottom = kFluidSize;

#define GL_TEXTURE_RECTANGLE_ARB          0x84F5

  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, normalTexture );
  glEnable( GL_TEXTURE_RECTANGLE_ARB );
  glColor3f(1,0,1);

  glBegin(GL_TRIANGLE_STRIP);
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

    //glTexCoord4f( vertex.tx, vertex.ty, vertex.tz, vertex.tw );
    glTexCoord2f( vertex.tx, vertex.ty );
    glVertex4f( vertex.x, vertex.y, vertex.z, vertex.w );
  }
  glEnd();

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
