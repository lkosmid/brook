#include <iostream>
#include<assert.h>
#include "cpu.hpp"
static void nothing (const std::vector<void*>&args,
                     unsigned int start,
                     unsigned int end){}
extern unsigned int knownTypeSize(__BRTStreamType);
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
    void CPUKernel::PushStream(Stream *s){
	unsigned int total_size=s->getTotalSize();
	if (extent==0)//this is necessary for reductions
          extent=total_size;//don't override output tho

        args.push_back(s);
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
    void CPUKernel::PushGatherStream(Stream *s){
        args.push_back(s);
    }
    void CPUKernel::PushReduce(void * data, __BRTStreamType type) {

       switch (type) {
       case __BRTSTREAM:
          args.push_back(NULL);
          reductions.push_back(ReductionArg(args.size(),type,*static_cast<const __BRTStream*>(data)));
          break;
       default:
          args.push_back(data);
          reductions.push_back(ReductionArg(args.size(),type,NULL));
       }

    }
    void CPUKernel::PushOutput(Stream *s){
        args.push_back(s);
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
        reductions.clear();
        args.clear();
#if 0
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
#endif
	extent=0;
    }
   //subMap is guaranteed that all reductions are actual values stored in args.
   //subMap is in charge of parallelizing threads where necessary.
    void CPUKernel::subMap(unsigned int begin, unsigned int end){
       (*func)(args,
               begin,
               end);//can do some fancy forking algorithm here
    }
    void CPUKernel::Map() {
       subMap(0,extent);

       Cleanup();
    }
    void CPUKernel::Release() {
	delete this;
    }
    CPUKernel::~CPUKernel() {

    }
}
