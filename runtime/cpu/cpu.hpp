#pragma once
#include "../brt.hpp"

namespace brook {
    class CPUKernel : public Kernel {
    public:
	CPUKernel(class CPURunTime * runtime, const void * source []);
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
	CPURunTime * runtime;
    };
    class CPUStream: public Stream {
    public:
	CPUStream (const char type[],int dims, int extents[]);
	virtual void Read(const void* inData);
	virtual void Write(void* outData);
	virtual void Release();
	virtual void *getData(){return data;}
	virtual unsigned int * getExtents(){return extents;}
	virtual unsigned int getDimension(){return dims;}
    protected:
	void * data;
	unsigned int * extents;
	unsigned int dims;		
	unsigned int stride;
	unsigned int totalsize;
	virtual ~CPUStream();
    };
    class CPURunTime: public RunTime {
    public:
	CPURunTime();
	virtual Kernel * CreateKernel(const void*[]);
	virtual Stream * CreateStream(const char type[], int dims, int extents[]);
	virtual ~CPURunTime(){}
    };
}
