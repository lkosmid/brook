// context.cpp
#pragma once

#include "core.hpp"
#include "context.hpp"

using namespace fibble;

Context* Context::create( Window* inWindow )
{
  Context* result = new Context( inWindow );
  if( result->initialize() )
    return result;
  delete result;
  return NULL;
}

Context::Context( Window* inWindow )
  : window( inWindow )
{
}

Context::~Context()
{
}

bool Context::initialize()
{
  HWND windowHandle = window->getWindowHandle();

	direct3d = Direct3DCreate9( D3D_SDK_VERSION );
  if( direct3d == NULL )
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

	HRESULT result = direct3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &deviceDesc, &device );
  if( FAILED(result) )
  {
    DX9Warn("Could not create Direct3D device.");
    return false;
  }

	// TIM: set up initial state
	result = device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
  DX9AssertResult( result, "SetRenderState failed" );

  return true;
}

void Context::clear()
{
  HRESULT result = device->Clear( 0, NULL,
    D3DCLEAR_TARGET,
//    D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
    0x00000000, 1.0f, 0 );
  DX9AssertResult( result, "Clear Failed" );
}

void Context::swap()
{
  HRESULT result = device->Present( NULL, NULL, NULL, NULL );
  DX9AssertResult( result, "Present Failed" );
}

bool Context::beginScene()
{
  HRESULT result = device->BeginScene();
  DX9AssertResult( result, "BeginScene Failed" );
  return true;
}

void Context::endScene()
{
  HRESULT result = device->EndScene();
  DX9AssertResult( result, "Present Failed" );
}

VertexShader* Context::createVertexShader( const std::string& inSource )
{
  return new VertexShader( this, inSource );
}

PixelShader* Context::createPixelShader( const std::string& inSource )
{
  return new PixelShader( this, inSource );
}

void Context::setVertexShader( VertexShader* inShader )
{
  IDirect3DVertexShader9* shader = inShader->getShaderHandle();
  HRESULT result = device->SetVertexShader( shader );
  if( FAILED(result) ) throw -1;
}

void Context::setPixelShader( PixelShader* inShader )
{
  IDirect3DPixelShader9* shader = inShader->getShaderHandle();
  HRESULT result = device->SetPixelShader( shader );
  if( FAILED(result) ) throw -1;
}
