#include "cpu.hpp"
namespace brook{
	const char * CPU_RUNTIME_STRING="CPU_RUNTIME";
    CPURunTime::CPURunTime() {

    }
    Kernel * CPURunTime::CreateKernel(const void* blah[]) {
	return new CPUKernel(blah);
    }
    Stream * CPURunTime::CreateStream(__BRTStreamType type,int dims, int extents[]) {
	return new CPUStream(type,dims,extents);
    }

}
