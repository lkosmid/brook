// runtime.hpp
#ifndef __RUNTIME_HPP__
#define __RUNTIME_HPP__

#include <brook/brt.hpp>

namespace brook {

  class Runtime {
  public:
    Runtime() {}

    virtual Kernel* CreateKernel(const void*[]) = 0;

    virtual Stream* CreateStream(unsigned int fieldCount, 
                                 const StreamType fieldTypes[],
                                 unsigned int dims, 
                                 const unsigned int extents[], 
                                 bool read_only=false) = 0;

    virtual Iter* CreateIter(StreamType type, 
                             unsigned int dims, 
                             const unsigned int extents[], 
                             const float ranges[])=0;
    virtual ~Runtime() {}

    // new entry points needed by C++ brook API
    virtual void finish() = 0;
    virtual void unbind() = 0;
    virtual void bind() = 0;
    virtual IWriteQuery* createWriteQuery() { return 0; }
    virtual IWriteMask* createWriteMask( int inY, int inX ) { return 0; }

    // TIM: hacky magick for raytracer
    virtual void hackEnableWriteMask() { assert(false); throw 1; }
    virtual void hackDisableWriteMask() { assert(false); throw 1; }
    virtual void hackSetWriteMask( Stream* ) { assert(false); throw 1; }
    virtual void hackBeginWriteQuery() { assert(false); throw 1; }
    virtual int hackEndWriteQuery() { assert(false); throw 1; }

    static Runtime* GetInstance( const char* inRuntimeName = 0, 
                                 void* inContextValue = 0, 
                                 bool addressTranslation = false );

  private:
    friend void brook::finalize();

    static Runtime*& GetInstanceRef();
    static Runtime* CreateInstance( const char* inRuntimeName, 
                                    void* inContextValue, 
                                    bool addressTranslation );
  };
}
#endif

