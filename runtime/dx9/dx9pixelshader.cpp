// dx9pixelshader.cpp
#include "dx9pixelshader.hpp"

#include "dx9.hpp"

using namespace brook;

DX9PixelShader::DX9PixelShader( DX9RunTime* inContext, const char* inSource )
{
	LPDIRECT3DDEVICE9 device = inContext->getDevice();
	HRESULT result;
	LPD3DXBUFFER codeBuffer;
  LPD3DXBUFFER errorBuffer;
	
	result = D3DXAssembleShader( inSource, strlen(inSource), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
  {
    const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
    DX9Fail( "DX9PixelShader failure - compile error\n%s", errorMessage );
  }
  DX9CheckResult( result );


	result = device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();
	DX9CheckResult( result );
}

DX9PixelShader::~DX9PixelShader()
{
}

DX9PixelShader* DX9PixelShader::create( DX9RunTime* inContext, const char* inSource )
{
	return new DX9PixelShader( inContext, inSource );
}

LPDIRECT3DPIXELSHADER9 DX9PixelShader::getHandle()
{
	return shaderHandle;
}
