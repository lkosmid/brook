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
       void subMap(unsigned int begin, unsigned int end);
       virtual void Release();
    protected:
       virtual ~CPUKernel();
       typedef void callable(const std::vector<void *>&args,
                             const std::vector<const unsigned int *>&extents,
                             const std::vector<unsigned int> &dims,
                             unsigned int start, 
                             unsigned int extent);
       typedef void nDcallable(const std::vector<void *>&args,
                               const std::vector<const unsigned int *>&extents,
                               const std::vector<unsigned int> &dims,
                               const unsigned int * start,
                               const unsigned int * extent);
       typedef void combinable(const std::vector<void*>&args,
                               const std::vector<const unsigned int *>&extents,
                               const std::vector<unsigned int> &dims,
                               unsigned int start);
       callable * func;
       combinable *combine;
       nDcallable *nDfunc;
       std::vector<void *> args;
       std::vector<const unsigned int *>extents;
       std::vector<unsigned int> dims;
       std::vector<brook::Stream*> inputs;
       std::vector<brook::Stream*>outputs;
       class ReductionArg {public:
          unsigned int which;
          __BRTStreamType type;
          brook::Stream *stream;
          ReductionArg(unsigned int w, __BRTStreamType s,brook::Stream *stream) {
             which=w;type=s;
             this->stream=stream;
          }
       };
       brook::Stream  * streamReduction;
       std::vector<ReductionArg> reductions;
       unsigned int totalsize;
       unsigned int dim;
       const unsigned int *extent;
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
  class CPUIter:public Iter {
  public:
    CPUIter(__BRTStreamType type, int dims, int extents[], float ranges[])
    :Iter(type){
      madeStream=makeStream(dims,extents,ranges);//now we always have this
    }
  };
    class CPURunTime: public brook::RunTime {
    public:
	CPURunTime();
	virtual Kernel * CreateKernel(const void*[]);
	virtual Stream * CreateStream(__BRTStreamType type, int dims, int extents[]);
	virtual Iter * CreateIter(__BRTStreamType type, 
                                  int dims, 
                                  int extents[],
                                  float ranges[]);
	virtual ~CPURunTime(){}
    };
}
#endif
