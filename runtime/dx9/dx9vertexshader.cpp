// dx9vertexshader.cpp
#include "dx9vertexshader.hpp"

#include "dx9.hpp"

using namespace brook;

DX9VertexShader::DX9VertexShader( DX9RunTime* inContext )
  : device(NULL), shaderHandle(NULL)
{
	device = inContext->getDevice();
  device->AddRef();
}

DX9VertexShader::~DX9VertexShader()
{
  if( shaderHandle != NULL )
    shaderHandle->Release();
  if( device != NULL )
    device->Release();
}

bool DX9VertexShader::initialize( const char* inSource )
{
  HRESULT result;
	LPD3DXBUFFER codeBuffer;
	LPD3DXBUFFER errorBuffer;
	
	result = D3DXAssembleShader( inSource, strlen(inSource), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
  {
    const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
    DX9WARN << "Vertex shader failed to compile:\n" << errorMessage;
    return false;
  }
  else if( FAILED(result) )
  {
    DX9WARN << "Vertex shader failed to compile.";
    return false;
  }

	result = device->CreateVertexShader( (DWORD*)codeBuffer->GetBufferPointer(), &shaderHandle );
	codeBuffer->Release();

  if( FAILED(result) )
  {
    DX9WARN << "Failed to allocate vertex shader.";
    return false;
  }
  return true;
}

DX9VertexShader* DX9VertexShader::create( DX9RunTime* inContext, const char* inSource )
{
  DX9PROFILE("DX9VertexShader::create")
  DX9VertexShader* result = new DX9VertexShader( inContext );
  if( result->initialize( inSource ) )
    return result;
  delete result;
  return NULL;
}
