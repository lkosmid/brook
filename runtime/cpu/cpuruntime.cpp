#include "cpu.hpp"
namespace brook{
	const char * CPU_RUNTIME_STRING="cpu";

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    CPURunTime::CPURunTime() {

    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    Kernel * CPURunTime::CreateKernel(const void* blah[]) {
	return new CPUKernel(blah);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    Stream * CPURunTime::CreateStream(
      int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]) {
	    return new CPUStream(fieldCount, fieldTypes,dims,extents);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    Iter * CPURunTime::CreateIter(__BRTStreamType type,
                                 int dims, 
                                 int extents[], 
                                 float ranges[]) {
       return new CPUIter(type,dims,extents,ranges);
    }

}
