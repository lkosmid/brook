#include <iostream>
#include<assert.h>
#include "cpu.hpp"
static void nothing (const std::vector<void*>&args,
                     const std::vector<const unsigned int *>&extents,
                     const std::vector<unsigned int> &dims,
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
                  std::cerr<<"CPUKernel failure - ";
                  std::cerr<<"no CPU program string found.";
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
            if (strcmp(src[i],"ndcpu")==0) {
               nDfunc=(nDcallable*)source[i+1];
            }
	}
        Cleanup();
    }
    void CPUKernel::PushStream(Stream *s){
	unsigned int total_size=s->getTotalSize();
	if (totalsize==0){//this is necessary for reductions
          totalsize=total_size;//don't override output tho
          dim=s->getDimension();
          extent= s->getExtents();
        }
        PushGatherStream(s);
    }
    void CPUKernel::PushConstant(const float &val){
        args.push_back(const_cast<float*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }
    void CPUKernel::PushConstant(const float2 &val){
        args.push_back(const_cast<float2*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }
    void CPUKernel::PushConstant(const float3 &val){
        args.push_back(const_cast<float3*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }
    void CPUKernel::PushConstant(const float4 &val){
        args.push_back(const_cast<float4*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }
    void CPUKernel::PushGatherStream(Stream *s){
        args.push_back(s->getData(brook::Stream::READ));
        extents.push_back(s->getExtents());
        dims.push_back(s->getDimension());
        inputs.push_back(s);
    }
    void CPUKernel::PushReduce(void * data, __BRTStreamType type) {
       if (type==__BRTSTREAM) {
          brook::Stream * stream = *(const __BRTStream *)data;
          reductions.push_back(ReductionArg(args.size(),type,stream));
          args.push_back(NULL);
          dims.push_back(stream->getDimension());
          extents.push_back(stream->getExtents());
          if (streamReduction) {
             AssertSameSize(reductions.back().stream,streamReduction);
          }else {
             streamReduction=reductions.back().stream;
          }
       }else {
          args.push_back(data);
          dims.push_back(0);
          extents.push_back(0);
          reductions.push_back(ReductionArg(args.size(),type,NULL));
       }

    }
    void CPUKernel::PushOutput(Stream *s){
        args.push_back(s->getData(brook::Stream::WRITE));
	totalsize=s->getTotalSize();
        dim=s->getDimension();
        extent= s->getExtents();
        dims.push_back(dim);
        extents.push_back(extent);
        outputs.push_back(s);
    }
    void CPUKernel::Cleanup() {
        reductions.clear();
        args.clear();
        extents.clear();
        dims.clear();
	totalsize=0;
        dim=0;
        extent=0;
        streamReduction=0;
        while (!inputs.empty()) {
           inputs.back()->releaseData(brook::Stream::READ);
           inputs.pop_back();
        }
        while (!outputs.empty()) {
           outputs.back()->releaseData(brook::Stream::WRITE);
           outputs.pop_back();
        }
    }
   //subMap is guaranteed that all reductions are actual values stored in args.
   //subMap is in charge of parallelizing threads where necessary.
    void CPUKernel::subMap(unsigned int begin, unsigned int end){
       (*func)(args,
               extents,
               dims,
               begin,
               end);//can do some fancy forking algorithm here
    }
    void CPUKernel::Map() {
       if (!streamReduction){
          subMap(0,totalsize);
       }else {
          unsigned int i;
          unsigned int rdim = streamReduction->getDimension();
          const unsigned int * rextent = streamReduction->getExtents();
          unsigned int total = streamReduction->getTotalSize();
          std::vector<ReductionArg>::iterator j;
          for (j=reductions.begin();j!=reductions.end();++j) {
             assert ((*j).type==__BRTSTREAM);
             args[(*j).which]=(*j).stream->getData(brook::Stream::WRITE);
          }
          unsigned int * buffer = (unsigned int *)
             malloc(2*dim*sizeof(unsigned int));
          unsigned int * e =buffer;
          memset (e,0,sizeof(unsigned int)*2*dim);
          unsigned int * mag = e+dim;
          for (i=0;i<dim;++i) {
             if (i<rdim)
                mag[i] = extent[i]/rextent[i];
             else
                mag[i] = extent[i];             
          }
          for (i=0;i<total;++i) {
             unsigned int k;
             for (j=reductions.begin();j!=reductions.end();++j) {
                args[(*j).which]=(char*)args[(*j).which]+
                   (*j).stream->getStride();
             }
             (*nDfunc)(args,extents,dims,e,mag);
             e[0]+=mag[0];
             for (k=0;k<rdim-1;++k) {
                if (e[k]>=extent[k]){
                   e[k]=0;
                   e[k+1]+=mag[k+1];                   
                }else break;
             }
          }
          free (e);
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
