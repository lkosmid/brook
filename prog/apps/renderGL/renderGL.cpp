// renderGL.cpp
#include "renderGL.hpp"

#include "RenderWindow.hpp"

RenderGLApplication::RenderGLApplication()
  : window(NULL)
{
}

RenderGLApplication::~RenderGLApplication()
{
}

int RenderGLApplication::run( int argc, char** argv )
{
  using namespace fibble;

  window = new RenderWindow();
  window->show();
  
  int result = runMessageLoop();

  delete window;

  return result;
}

void RenderGLApplication::handleIdle() {
  window->handleIdle();
}

