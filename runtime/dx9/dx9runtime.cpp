#include <dx9.hpp>
#include <dx9base.hpp>

#include "dx9window.hpp"

DX9RunTime::DX9RunTime() {
  // XXX: TO DO
  // TIM: initialize D3D
  DX9Trace("DX9RunTime::DX9RunTime");

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
	deviceDesc.EnableAutoDepthStencil = TRUE;
	deviceDesc.AutoDepthStencilFormat = D3DFMT_D24S8;

	HRESULT result = direct3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF/*D3DDEVTYPE_HAL*/, windowHandle,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &deviceDesc, &device );
	DX9CheckResult( result );

	// TIM: set up initial state
	device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
//	device->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );
}

__BrookKernel * DX9RunTime::LoadKernel(const char* source[]) {
  return new DX9Kernel( this, source );
}

__BrookStream * DX9RunTime::CreateStream(const char type[], int dims, int extents[]) {
  // XXX: TO DO
  return new DX9Stream( this, type, dims, extents );
}

DX9RunTime::~DX9RunTime() {
  // Does nothing
  // TIM: finalize D3D
}
