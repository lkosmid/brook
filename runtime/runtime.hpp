// runtime.hpp
#ifndef __RUNTIME_HPP__
#define __RUNTIME_HPP__

#include <brt.hpp>
#include <kerneldesc.hpp>

namespace brook {

  class RunTime {
  public:
    RunTime() {}
    virtual Kernel* CreateKernel(const void*[]) = 0;
    virtual Stream* CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]) = 0;
    virtual Iter * CreateIter(__BRTStreamType, 
                              int dims, 
                              int extents[], 
                              float ranges[])=0;
    virtual ~RunTime() {}

    static RunTime* GetInstance( const char* inRuntimeName = 0, 
                                 void* inContextValue = 0, 
                                 bool addressTranslation = false );

  private:
    static RunTime* CreateInstance( const char* inRuntimeName, 
                                    void* inContextValue, 
                                    bool addressTranslation );
  };
}
#endif
