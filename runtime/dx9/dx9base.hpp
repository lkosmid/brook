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
#include "../logger.hpp"

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

  #define DX9PROFILE( __name ) BROOK_PROFILE( __name )

  #define DX9LOG( __level ) \
    BROOK_LOG( __level ) << "DX9 - "

  #define DX9LOGPRINT( __level ) \
    BROOK_LOG_PRINT( __level )

  #define DX9WARN \
    std::cerr << "Brook Runtime (dx9) - "

  inline void DX9AssertImpl( const char* fileName, int lineNumber, const char* comment )
  {
    DX9WARN << fileName << "(" << lineNumber << "): " << comment << std::endl;
    assert(false);
    exit(1);
  }

  #define DX9AssertResult( _result, _message ) \
    if(SUCCEEDED(_result)) {} else DX9AssertImpl( __FILE__, __LINE__, _message )

  #define DX9Assert( _condition, _message ) \
    if(_condition) {} else DX9AssertImpl( __FILE__, __LINE__, _message )
}