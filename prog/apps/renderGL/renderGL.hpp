// renderDX.hpp
#pragma once

#include "core.hpp"

class RenderWindow;

class RenderGLApplication : public fibble::Application
{
public:
  RenderGLApplication();
  ~RenderGLApplication();

  int run( int argc, char** argv );

private:
  void handleIdle();

  RenderWindow* window;
};