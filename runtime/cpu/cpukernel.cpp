#include<assert.h>
#include "cpu.hpp"
static void nothing (const std::vector<void*>&args,unsigned int start,unsigned int end){}
namespace brook{
    CPUKernel::CPUKernel(const void * source []){
        const char ** src= (const char**)(source);
	for (unsigned int i=0;;i+=2) {
	    if (src[i]==NULL||src[i+1]==NULL){
		func=&nothing;
		break;
	    }
	    if (strcmp(src[i],"cpu")==0){
		func = (callable*)source[i+1];
		break;
	    }
	}
        extent=0;
    }
    void CPUKernel::PushStream(const Stream *s){
        args.push_back(static_cast<const CPUStream*>(s)->getData());
	unsigned int total_size=static_cast<const CPUStream*>(s)->getTotalSize();
	if (extent==0)
	    extent=total_size;
	else
	    assert(extent==total_size);
    }
    void CPUKernel::PushConstant(const float &val){
        args.push_back(const_cast<float*>(&val));
    }
    void CPUKernel::PushConstant(const float2 &val){
        args.push_back(const_cast<float2*>(&val));
    }
    void CPUKernel::PushConstant(const float3 &val){
        args.push_back(const_cast<float3*>(&val));
    }
    void CPUKernel::PushConstant(const float4 &val){
        args.push_back(const_cast<float4*>(&val));
    }
    void CPUKernel::PushGatherStream(const Stream *s){
        args.push_back(const_cast<Stream*>(s));
    }
    void CPUKernel::PushOutput(const Stream *s){
        this->PushStream(s);
    }
    void CPUKernel::Map(){
	(*func)(args,0,extent);
        args.clear();
    }
}
