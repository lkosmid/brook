// dx9base.hpp
#pragma once

#include <assert.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <brt.hpp>

namespace brook {

  class DX9RunTime;
  class DX9Stream;
  class DX9Kernel;
  class DX9Texture;
  class DX9Window;
  class DX9VertexShader;
  class DX9PixelShader;

  // TIM: 'helper' struct for defining the bounds
  // of a stream in its texture:
  struct DX9Rect
  {
    DX9Rect() {}
    DX9Rect( float inLeft, float inTop, float inRight, float inBottom )
      : left(inLeft), top(inTop), right(inRight), bottom(inBottom) {}
    
    operator float*() { return (float*)this; }
      
    operator const float*() const { return (const float*)this; }

    float left, top, right, bottom;
  };

  inline void DX9Trace( const char* inFormat, ... )
  {
    static FILE* file = fopen( "./DX9RuntimeLog.txt", "w" );

    va_list args;
    va_start( args, inFormat );
    vfprintf( file, inFormat, args );
    fprintf( file, "\n" );
    fflush( file );
    va_end(args);
  }

  inline void DX9Fail( const char* inFormat, ... )
  {
    va_list args;
    va_start( args, inFormat );
    vfprintf( stderr, inFormat, args );
    fflush( stderr );
    va_end(args);
    exit(-1);
  }

  inline void DX9CheckResultImpl( HRESULT result, const char* fileName, int lineNumber )
  {
    if( !FAILED(result) ) return;
    DX9Fail( "HRESULT failure - %s:(%d)", fileName, lineNumber );
  }

  #define DX9CheckResult( _result ) \
    DX9CheckResultImpl( _result, __FILE__, __LINE__ );
}