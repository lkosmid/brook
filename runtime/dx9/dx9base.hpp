// dx9base.hpp
#pragma once

#include <assert.h>
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

class DX9RunTime;
class DX9Stream;
class DX9Kernel;
class DX9Texture;
class DX9Window;

inline void DX9Trace( const char* inFormat, ... )
{
  va_list args;
  va_start( args, inFormat );
  vfprintf( stdout, inFormat, args );
  fprintf( stdout, "\n" );
  va_end(args);
  fflush( stdout );
}

inline void DX9CheckResultImpl( HRESULT result, const char* fileName, int lineNumber )
{
  if( !FAILED(result) ) return;
  DX9Trace( "HRESULT failure - %s:(%d)", fileName, lineNumber );
  assert( false && "DX9 returned failure");
}

#define DX9CheckResult( _result ) \
  DX9CheckResultImpl( _result, __FILE__, __LINE__ );