#include <iostream>
#include<assert.h>
#include "cpu.hpp"
static void nothing (const std::vector<void*>&args,
                     unsigned int start,
                     unsigned int end){}
extern unsigned int knownTypeSize(__BRTStreamType);
void AssertSameSize (brook::Stream *reduction,brook::Stream *newreduction) {
             unsigned int dims = reduction->getDimension();
             unsigned int newdims = newreduction->getDimension();
             assert (dims==newdims);
             const unsigned int * extents=reduction->getExtents();
             const unsigned int *newextents=newreduction->getExtents();
             for (unsigned int i=0;i<dims;++i) {
                assert (extents[i]==newextents[i]);
             }
}


namespace brook{
    CPUKernel::CPUKernel(const void * source []){
        const char ** src= (const char**)(source);
        combine=0;func=0;
	for (unsigned int i=0;;i+=2) {
	    if (src[i]==NULL){
               if (!func){
                  func=&nothing;
                  std::cerr<<"CPUKernel failure - no CPU program string found.";
                  std::cerr <<std::endl;
               }
               break;
	    }
	    if (strcmp(src[i],"cpu")==0){
		func = (callable*)source[i+1];
	    } else
            if (strcmp(src[i],"combine")==0) {
               combine=(combinable*)source[i+1];
            }
	}
        Cleanup();
    }
    void CPUKernel::PushStream(Stream *s){
	unsigned int total_size=s->getTotalSize();
	if (totalsize==0){//this is necessary for reductions
          totalsize=total_size;//don't override output tho
          dim=s->getDimension();
          extents= s->getExtents();
        }
        args.push_back(s);
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
          if (streamReduction) {
             AssertSameSize(reductions.back().stream,streamReduction);
          }else {
             streamReduction=reductions.back().stream;
          }
          break;
       default:
          args.push_back(data);
          reductions.push_back(ReductionArg(args.size(),type,NULL));
       }

    }
    void CPUKernel::PushOutput(Stream *s){
        args.push_back(s);
	totalsize=s->getTotalSize();
        dim=s->getDimension();
        extents= s->getExtents();
    }
    void CPUKernel::Cleanup() {
        reductions.clear();
        args.clear();
	totalsize=0;
        dim=0;
        extents=0;
        streamReduction=0;
    }
   //subMap is guaranteed that all reductions are actual values stored in args.
   //subMap is in charge of parallelizing threads where necessary.
    void CPUKernel::subMap(unsigned int begin, unsigned int end){
       (*func)(args,
               begin,
               end);//can do some fancy forking algorithm here
    }
    void CPUKernel::Map() {
       if (!streamReduction){
          subMap(0,totalsize);
       }else {
          unsigned int rdim = streamReduction->getDimension();
          const unsigned int * rextents = streamReduction->getExtents();
          unsigned int total = streamReduction->getTotalSize();
          std::vector<ReductionArg>::iterator j;
          for (j=reductions.begin();j!=reductions.end();++j) {
             args[(*j).which]=(*j).stream->getData(brook::Stream::WRITE);
          }
          for (unsigned int i=0;i<total;++i) {
             for (j=reductions.begin();j!=reductions.end();++j) {
                args[(*j).which]=(char*)args[(*j).which]+(*j).stream->getStride();
             }
             
          }
          for (j=reductions.begin();j!=reductions.end();++j) {
             (*j).stream->releaseData(brook::Stream::WRITE);
          }
       }
       Cleanup();
    }
    void CPUKernel::Release() {
	delete this;
    }
    CPUKernel::~CPUKernel() {

    }
}
