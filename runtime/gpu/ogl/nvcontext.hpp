// nvcontext.hpp
#ifndef NV_CONTEXT_HPP
#define NV_CONTEXT_HPP

#include "../gpucontext.hpp"

namespace brook {

  class NVTexture;
  class NVContext;

  class NVTexture : public OGLTexture {
  public:

    NVTexture ( unsigned int inWidth, 
                unsigned int inHeight, 
                TextureFormat inFormat);
    
    virtual int nativeFormat() const { return _nativeFormat; }

  private:
    int _nativeFormat;
  };


  class NVContext : public OGLContext
  {
  public:
    
    NVContext();

    TextureHandle 
    createTexture2D( unsigned int inWidth, 
                     unsigned int inHeight, 
                     TextureFormat inFormat);
  };
}

#endif
