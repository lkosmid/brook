#pragma once
#include "../brt.hpp"
namespace brook {
    class CPUKernel : public Kernel {
    public:
	CPUKernel(CPURunTime * runtime, const void * source []);
	virtual void PushStream(const Stream *s);
	virtual void PushConstant(const float &val);  
	virtual void PushConstant(const float2 &val);  
	virtual void PushConstant(const float3 &val); 
	virtual void PushConstant(const float4 &val);
	virtual void PushGatherStream(const Stream *s);
	virtual void PushOutput(const Stream *s);
	virtual void Map();
	virtual void Release() {}
    private:
	virtual ~CPUKernel();
	CPURunTime * cpu;
    };
    class CPUStream: public Stream {
    public:
	CPUStream (CPURunTime *runtime, const char type[],int dims, int extents[]);
	virtual void Read(const void* inData);
	virtual void Write(void* outData);
	virtual void Release();
	virtual void *getData(){return data;}
	virtual int * getExtents(){return extents;}
    private:
	int dims;	
	int * extents;
	void * data;	
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
