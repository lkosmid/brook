// dx9vertexshader.hpp
#pragma once

#include "dx9base.hpp"

class DXContext;

class DX9VertexShader
{
public:
	static DX9VertexShader* create( DX9RunTime* inContext, const char* inSource );
	~DX9VertexShader();

	IDirect3DVertexShader9* getHandle();

private:
	DX9VertexShader( DX9RunTime* inContext, const char* inSource );

	IDirect3DVertexShader9* shaderHandle;
};