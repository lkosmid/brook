// dx9vertexshader.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class VertexShader
  {
  public:
	  ~VertexShader();

  private:
    friend class Context;
	  VertexShader( Context* inContext, const std::string& inSource );
  };

}