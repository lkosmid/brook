#ifndef _BROOK_CPU_HPP
#define _BROOK_CPU_HPP
#include <vector>
#include "../brt.hpp"

namespace brook {
    class CPUKernel : public Kernel {
    public:
	CPUKernel(const void * source []);
	virtual void PushStream(const Stream *s);
	virtual void PushConstant(const float &val);  
	virtual void PushConstant(const float2 &val);  
	virtual void PushConstant(const float3 &val); 
	virtual void PushConstant(const float4 &val);
	virtual void PushGatherStream(const Stream *s);
	virtual void PushOutput(const Stream *s);
	virtual void Map();
	virtual void Release();
    protected:
	virtual ~CPUKernel();
	typedef void callable(const std::vector<void *>&args,unsigned int start, unsigned int end);
	callable * func;
	std::vector<void *> args;
	unsigned int extent;
    };
    class CPUStream: public Stream {
    public:
	CPUStream (const char type[],int dims, int extents[]);
	virtual void Read(const void* inData);
	virtual void Write(void* outData);
	virtual void Release();
	virtual void *getData(){return data;}
	virtual void *getData()const{return data;}	
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
	virtual Stream * CreateStream(const char type[], int dims, int extents[]);
	virtual ~CPURunTime(){}
    };
}
#endif
