// pixelshader.cpp

#include "core.hpp"
#include "pixelshader.hpp"

using namespace fibble;

PixelShader::PixelShader( Context* inContext, const std::string& inSource )
  : device(NULL), shaderHandle(NULL)
{
  device = inContext->getDevice();
  device->AddRef();

  // assemble the shader
  HRESULT result;
	LPD3DXBUFFER codeBuffer;
	LPD3DXBUFFER errorBuffer;
	
	result = D3DXAssembleShader( inSource.c_str(), inSource.size(), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
  {
    const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
    DX9Warn( "Pixel shader failed to compile:\n%s", errorMessage );
    throw -1;
  }
  else if( FAILED(result) )
  {
    DX9Warn( "Pixel shader failed to compile." );
    throw -1;
  }

	result = device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();

  if( FAILED(result) )
  {
    DX9Warn( "Failed to allocate pixel shader." );
    throw -1;
  }
}

PixelShader::~PixelShader()
{
  if( shaderHandle != NULL )
    shaderHandle->Release();
  if( device != NULL )
    device->Release();
}
