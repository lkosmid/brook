// renderDX.cpp
#include "renderDX.hpp"

#include "RenderWindow.hpp"

static const int kFluidSize = 128;

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
//"mov r0, r0.x\n"
"mov oC0, r0\n"
;


RenderDXApplication::RenderDXApplication()
  : window(NULL)
{
}

RenderDXApplication::~RenderDXApplication()
{
}

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

int RenderDXApplication::run( int argc, char** argv )
{
  using namespace fibble;

  window = new RenderWindow();
  window->show();
  
  int result = runMessageLoop();

  delete window;

  return result;
}

void RenderDXApplication::handleIdle() {
  window->handleIdle();
}

