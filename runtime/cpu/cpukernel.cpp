#include <iostream>
#include<assert.h>
#include "cpu.hpp"
static void nothing (const std::vector<void*>&args,unsigned int start,unsigned int end){}
namespace brook{
    CPUKernel::CPUKernel(const void * source []){
        const char ** src= (const char**)(source);
	writeOnly=NULL;
	for (unsigned int i=0;;i+=2) {
	    if (src[i]==NULL){
		func=&nothing;
		std::cerr<<"CPUKernel failure - no CPU program strnig found."<<std::endl;		
		break;
	    }
	    if (strcmp(src[i],"cpu")==0){
		func = (callable*)source[i+1];
		break;
	    }
	}
        extent=0;
    }
    void CPUKernel::PushStream(const Stream *constStream){
        Stream * s = const_cast<Stream*>(constStream);
        args.push_back(s->getData(Stream::READ));
	unsigned int total_size=s->getTotalSize();
	if (extent==0)
	    extent=total_size;
	else
	    assert(extent==total_size);
        
	readOnly.push_back(s);
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
    void CPUKernel::PushOutput(const Stream *constStream){
        Stream * s = const_cast<Stream*>(constStream);
        args.push_back(s->getData(Stream::WRITE));
	unsigned int total_size=s->getTotalSize();
	if (extent==0)
	    extent=total_size;
	else
	    assert(extent==total_size);       
	if (writeOnly==NULL)
	  writeOnly=s;
	else
	  writeOnlies.push_back(s);
    }
    void CPUKernel::Cleanup() {
        args.clear();
	while (!writeOnlies.empty()){
	  writeOnlies.back()->releaseData(Stream::WRITE);
	  writeOnlies.pop_back();
	}
	if (writeOnly)
	  writeOnly->releaseData(Stream::WRITE);
	writeOnly=NULL;
	while (!readOnly.empty()) {
	  readOnly.back()->releaseData(Stream::READ);
	  readOnly.pop_back();
	}
	extent=0;
    }
    void CPUKernel::Map(){
	(*func)(args,0,extent);//can do some fancy forking algorithm here
	Cleanup();
    }
    void CPUKernel::Release() {
	delete this;
    }
    CPUKernel::~CPUKernel() {

    }
}
