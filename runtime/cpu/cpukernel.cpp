#include "cpu.hpp"
namespace brook{
    CPUKernel::CPUKernel(const void * source []){
	this->func = (callable *)source[5];
    }
    void CPUKernel::PushStream(const Stream *s){
		args.push_back(static_cast<const CPUStream*>(s)->getData());
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

		args.clear();
    }
    void CPUKernel::Release() {

    }
    CPUKernel::~CPUKernel(){
	
    }
}
