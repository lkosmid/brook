// dx9vertexshader.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class VertexShader
  {
  public:
     ~VertexShader();
     void bind();

  private:
     friend class Context;
     VertexShader( Context* inContext, const std::string& inSource );

     unsigned int _id;
  };

}
