// runtime.hpp
#ifndef __RUNTIME_HPP__
#define __RUNTIME_HPP__

#include <brt.hpp>

namespace brook {

  class Runtime {
  public:
    Runtime() {}

    virtual Kernel* CreateKernel(const void*[]) = 0;

    virtual Stream* CreateStream(unsigned int fieldCount, 
                                 const StreamType fieldTypes[],
                                 unsigned int dims, 
                                 const unsigned int extents[]) = 0;

    virtual Iter* CreateIter(StreamType type, 
                             unsigned int dims, 
                             const unsigned int extents[], 
                             const float ranges[])=0;
    virtual ~Runtime() {}

    static Runtime* GetInstance( const char* inRuntimeName = 0, 
                                 void* inContextValue = 0, 
                                 bool addressTranslation = false );

  private:
    static Runtime* CreateInstance( const char* inRuntimeName, 
                                    void* inContextValue, 
                                    bool addressTranslation );
  };
}
#endif

