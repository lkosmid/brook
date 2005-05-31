// dx9pixelshader.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class PixelShader
  {
  public:
     ~PixelShader();
     void bind();

  private:
    friend class Context;
    PixelShader( Context* inContext, const std::string& inSource );
    unsigned int _id;
  };

}
