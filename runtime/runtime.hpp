// runtime.hpp
#ifndef __RUNTIME_HPP__
#define __RUNTIME_HPP__

#include <brt.hpp>

namespace brook {

  class RunTime {
  public:
    RunTime() {}
    virtual Kernel* CreateKernel(const void*[]) = 0;
    virtual Stream* CreateStream(const char type[], int dims, int extents[]) = 0;
    virtual ~RunTime() {}

    static RunTime* GetInstance();

  private:
    static RunTime* CreateInstance();
  };
}
#endif
