// window.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class Application
  {
  protected:
	  Application();
	  ~Application();

    int runMessageLoop();

    virtual void handleIdle() {}
  };
}