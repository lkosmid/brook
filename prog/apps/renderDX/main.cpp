// main.cpp

#include <iostream>
#include <stdlib.h>

#include "core.hpp"
#include "renderDX.hpp"

#include <brook/brook.hpp>

int main( int argc, char** argv )
{
  RenderDXApplication application;
  return application.run( argc, argv );
}
