#include "cpu.hpp"
namespace brook{
    CPURunTime::CPURunTime() {

    }
    Kernel * CPURunTime::CreateKernel(const void* blah[]) {
	return new CPUKernel(blah);
    }
    Stream *CreateStream(const char type[],int dims, int extents[]) {
	return new CPUStream(type,dims,extents);
    }

}
