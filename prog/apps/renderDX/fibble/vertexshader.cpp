// vertexshader.cpp

#include "core.hpp"
#include "vertexshader.hpp"

using namespace fibble;

VertexShader::VertexShader( Context* inContext, const std::string& inSource )
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
    DX9Warn( "Vertex shader failed to compile:\n%s", errorMessage );
    throw -1;
  }
  else if( FAILED(result) )
  {
    DX9Warn( "Vertex shader failed to compile." );
    throw -1;
  }

	result = device->CreateVertexShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();

  if( FAILED(result) )
  {
    DX9Warn( "Failed to allocate vertex shader." );
    throw -1;
  }
}

VertexShader::~VertexShader()
{
  if( shaderHandle != NULL )
    shaderHandle->Release();
  if( device != NULL )
    device->Release();
}
