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

#include "../profiler.hpp"

// uncomment this line to have the DX9
// runtime spit out
// lots of debug trace information
// #define BROOK_DX9_TRACE

// uncomment this line and the one above
// to make the DX9 runtime spit out
// temporary results during reductions
// #define BROOK_DX9_TRACE_REDUCE

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

  // A 'fattened' version of the DX9Rect structure
  // that allows us to specify arbitrary float4
  // values at the corners
  struct DX9FatRect
  {
    DX9FatRect() {}
    DX9FatRect( const DX9Rect& inRect ) {
      *this = inRect;
    }
    
    const DX9FatRect& operator=( const DX9Rect& inRect ) {
      for( int i = 0; i < 4; i++ ) {
        int xIndex = (i&0x01) ? 0 : 2;
        int yIndex = (i&0x02) ? 3 : 1;

        vertices[i].x = inRect[xIndex];
        vertices[i].y = inRect[yIndex];
        vertices[i].z = 0.0f;
        vertices[i].w = 1.0f;
      }
      return *this;
    }

    float4 vertices[4];
  };

  inline void DX9Spew( const char* inFormat, va_list args )
  {
    static FILE* file = fopen( "./DX9RuntimeLog.txt", "w" );
    vfprintf( file, inFormat, args );
    fflush( file );
  }

#if defined(BROOK_DX9_TRACE)
  inline void DX9Print( const char* inFormat, ... )
  {
    va_list args;
    va_start( args, inFormat );
    DX9Spew( inFormat, args );
    va_end(args);
  }

  inline void DX9Trace( const char* inFormat, ... )
  {
    static FILE* file = fopen( "./DX9RuntimeLog.txt", "w" );

    va_list args;
    va_start( args, inFormat );
    DX9Spew( inFormat, args );
    va_end(args);
    DX9Print( "\n" );
  }
#else
  inline void DX9Print(...) {}
  inline void DX9Trace(...) {}
#endif

  inline void DX9Warn( const char* inFormat, ... )
  {
    va_list args;
    va_start( args, inFormat );
    fprintf( stderr, "Brook Runtime (DirectX 9) Error:\n" );
    vfprintf( stderr, inFormat, args );
    fprintf( stderr, "\n" );
    fflush( stderr );
    va_end(args);
  }

  inline void DX9AssertImpl( const char* fileName, int lineNumber, const char* comment )
  {
    DX9Warn( "%s(%d): %s", fileName, lineNumber, comment );
    exit(1);
  }

  #define DX9AssertResult( _result, _message ) \
    if(SUCCEEDED(_result)) {} else DX9AssertImpl( __FILE__, __LINE__, _message )

  #define DX9Assert( _condition, _message ) \
    if(_condition) {} else DX9AssertImpl( __FILE__, __LINE__, _message )
}