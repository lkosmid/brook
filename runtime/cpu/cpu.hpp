#ifndef _BROOK_CPU_HPP
#define _BROOK_CPU_HPP
#include <vector>
#include "../runtime.hpp"

namespace brook {
	extern const char* CPU_RUNTIME_STRING;	
    class CPUKernel : public Kernel {
    public:
       CPUKernel(const void * source []);
       virtual void PushStream(Stream *s);
       virtual void PushConstant(const float &val);  
       virtual void PushConstant(const float2 &val);  
       virtual void PushConstant(const float3 &val); 
       virtual void PushConstant(const float4 &val);
       virtual void PushGatherStream(Stream *s);
       virtual void PushReduce (void * val, __BRTStreamType type);
       virtual void PushOutput(Stream *s);
       virtual void Map();
       virtual void Release();
    protected:
       virtual ~CPUKernel();
       typedef void callable(const std::vector<void *>&args,
                             unsigned int start, 
                             unsigned int end);
       typedef void combinable (const std::vector<void*>&args,
                                unsigned int start);
       callable * func;
       std::vector<void *> args;
       class ReductionArg {public:
          unsigned int which;
          __BRTStreamType type;
          ReductionArg(unsigned int w, __BRTStreamType s) {
             which=w;type=s;
          }
       };
       std::vector<ReductionArg> reductions;
       Stream * writeOnly;
       std::vector<Stream *> readOnly;
       std::vector<Stream *> writeOnlies;
       unsigned int extent;
       void Cleanup();
    };
    class CPUStream: public Stream {
    public:
	CPUStream (__BRTStreamType type ,int dims, int extents[]);
	virtual void Read(const void* inData);
	virtual void Write(void* outData);
	virtual void Release();
	virtual void *getData(unsigned int flags) {return data;}
        virtual void releaseData(unsigned int flags){}
	virtual const unsigned int * getExtents() const{return extents;}
	virtual unsigned int getDimension() const{return dims;}
        virtual unsigned int getTotalSize()const{return totalsize;}	
    protected:
	void * data;
	unsigned int * extents;
	unsigned int dims;		
	unsigned int stride;
	unsigned int totalsize;
	virtual ~CPUStream();
    };
    class CPURunTime: public brook::RunTime {
    public:
	CPURunTime();
	virtual Kernel * CreateKernel(const void*[]);
	virtual Stream * CreateStream(__BRTStreamType type, int dims, int extents[]);
	virtual ~CPURunTime(){}
    };
}
#endif
