// renderDX.hpp
#pragma once

#include "core.hpp"

class RenderWindow;

class RenderDXApplication : public fibble::Application
{
public:
  RenderDXApplication();
  ~RenderDXApplication();

  int run( int argc, char** argv );

private:
  void handleIdle();

  RenderWindow* window;
};