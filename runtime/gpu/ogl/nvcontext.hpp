// nvcontext.hpp
#ifndef NV_CONTEXT_HPP
#define NV_CONTEXT_HPP

#include "oglcontext.hpp"

namespace brook {

  class NVTexture : public OGLTexture {
  public:

    NVTexture ( unsigned int inWidth, 
                unsigned int inHeight, 
                GPUContext::TextureFormat inFormat);
    
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

    bool isValidShaderNameString (const char *name) const;


  protected:
    virtual void getVendorAttribs(const int   (**iAttrib)[4][16],
                                  const float (**fAttrib)[4][16],
                                  const int   (**piAttrib)[4][16]);
  };
}

#endif
