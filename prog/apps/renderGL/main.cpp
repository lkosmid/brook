// main.cpp

#include <iostream>
#include <stdlib.h>

#include "core.hpp"
#include "renderGL.hpp"

#include <brook/brook.hpp>

int main( int argc, char** argv )
{
  RenderGLApplication application;
  return application.run( argc, argv );
}
