// core.hpp
#pragma once

#include <assert.h>
#include <windows.h>
//#include <d3d9.h>
//#include <d3dx9.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>

#include "core.hpp"

#include "application.hpp"
#include "window.hpp"
#include "vertexshader.hpp"
#include "pixelshader.hpp"
#include "context.hpp"
/*
static inline void DX9Warn( const char* inFormat, ... ) {
}

static inline void DX9AssertResult( HRESULT inResult, const char* inMessage )
{
  if( !FAILED(inResult) ) return;
  std::cerr << inMessage << std::endl;
  abort();
}
*/

static inline void GLFail( const char* inMessage )
{
  std::cerr << inMessage << std::endl;
  abort();
}

#define GLAssert( __condition, __message ) \
  if( __condition ) {} else GLFail( __message );
