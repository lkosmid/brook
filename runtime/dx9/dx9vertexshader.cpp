// dx9vertexshader.cpp
#include "dx9vertexshader.hpp"

#include "dx9.hpp"

DX9VertexShader::DX9VertexShader( DX9RunTime* inContext, const char* inSource )
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

	result = device->CreateVertexShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();
	DX9CheckResult( result );
}

DX9VertexShader::~DX9VertexShader()
{
}

DX9VertexShader* DX9VertexShader::create( DX9RunTime* inContext, const char* inSource )
{
	return new DX9VertexShader( inContext, inSource );
}

LPDIRECT3DVERTEXSHADER9 DX9VertexShader::getHandle()
{
	return shaderHandle;
}
