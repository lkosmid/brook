// dx9pixelshader.cpp
#include "dx9pixelshader.hpp"

#include "dx9.hpp"

using namespace brook;

DX9PixelShader::DX9PixelShader( DX9RunTime* inContext )
  : device(NULL), shaderHandle(NULL)
{
  device = inContext->getDevice();
  device->AddRef();
}

DX9PixelShader::~DX9PixelShader()
{
  if( shaderHandle != NULL )
    shaderHandle->Release();
  if( device != NULL )
    device->Release();
}

bool DX9PixelShader::initialize( const char* inSource )
{
	HRESULT result;
	LPD3DXBUFFER codeBuffer;
  LPD3DXBUFFER errorBuffer;
	
	result = D3DXAssembleShader( inSource, strlen(inSource), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
  {
    const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
    DX9WARN << "Pixel shader failed to compile:\n" << errorMessage;
    return false;
  }
  else if( FAILED(result) )
  {
    DX9WARN << "Pixel shader failed to compile.";
    return false;
  }


	result = device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();

	if( FAILED(result) )
  {
    DX9WARN << "Failed to allocate pixel shader.";
    return false;
  }
  return true;
}

DX9PixelShader* DX9PixelShader::create( DX9RunTime* inContext, const char* inSource )
{
  DX9PROFILE("DX9PixelShader::create")
	DX9PixelShader* result = new DX9PixelShader( inContext );
  if( result->initialize( inSource ) )
    return result;
  delete result;
  return NULL;
}

