#ifndef _BROOK_CPU_HPP
#define _BROOK_CPU_HPP
#include <vector>
#include "../runtime.hpp"

namespace brook {
    extern const char* CPU_RUNTIME_STRING;	

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    class CPUKernel : public Kernel {
    public:
       CPUKernel(const void * source []);
       virtual void PushStreamInterface(StreamInterface * s);
       virtual void PushGatherStreamInterface(StreamInterface * s);
       virtual void PushStream(Stream *s);
       virtual void PushConstant(const float &val);  
       virtual void PushConstant(const float2 &val);  
       virtual void PushConstant(const float3 &val); 
       virtual void PushConstant(const float4 &val);
       virtual void PushIter(class Iter * i);
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
       std::vector<brook::StreamInterface*> inputs;
       std::vector<brook::StreamInterface*>outputs;

       // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
       class ReductionArg {public:
          // which argument is the reduction in question
          unsigned int which;
          // what type is this argument (so we can determine size)
          __BRTStreamType type;
          // is this a reduction stream?
          brook::Stream *stream;
          // create reduction arg
          ReductionArg(unsigned int w, 
                       __BRTStreamType s,
                       brook::Stream *stream) {
             which=w;type=s;
             this->stream=stream;
          }
       };
       brook::Stream  * streamReduction;
       std::vector<ReductionArg> reductions;
       unsigned int totalsize;
       unsigned int iteroutsize;
       unsigned int dim;
       const unsigned int *extent;
       bool multiThread;
       void Cleanup();
       void ThreadMap(unsigned int numThreads);
       void ReduceToStream(std::vector<void *>&args,
                           unsigned int cur,
                           unsigned int curfinal,
                           const unsigned int * extent,
                           unsigned int rdim,
                           unsigned int *mapbegin,
                           const unsigned int * mag)const;

       // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
       // Maps arguments to ReduceToStream
       struct reduceToStreamInput:public std::vector<void*>{
          const CPUKernel * thus;
          unsigned int cur,curfinal;
          const unsigned int *extent;
          unsigned int rdim;
          unsigned int * mapbegin;
          const unsigned int * mag;             
          reduceToStreamInput (const CPUKernel * thus,
                               const std::vector<void*> &args,
                               unsigned int cur,
                               unsigned int curfinal,
                               const unsigned int *extent,
                               unsigned int rdim,
                               unsigned int *begin,
                               const unsigned int * mag) 

             :std::vector<void*>(args),thus(thus),
              cur(cur),curfinal(curfinal),
              extent(extent),rdim(rdim),mapbegin(begin),mag(mag)
          {}
       };
       
       //frees reduceToStreamInput, input
       static void * staticReduceToStream(void * inp);

       // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
       // Maps arguments to staticSubMap
       class subMapInput:public std::vector<void*> {
       public:
          subMapInput(const CPUKernel *thus,
                      const std::vector<void *> &args,
                      unsigned int begin,
                      unsigned int end):
             std::vector<void*>(args),thus(thus),mapbegin(begin),mapend(end)
          {}
          const CPUKernel * thus;
          unsigned int mapbegin;
          unsigned int mapend;             
       };
       static void * staticSubMap(void * inp);
    };

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    class CPUStream: public Stream {
    public:
	CPUStream (__BRTStreamType type ,int dims, const int extents[]);
	virtual void Read(const void* inData);
	virtual void Write(void* outData);
	virtual void Release();
	virtual void *getData(unsigned int flags) {return data;}
        virtual void releaseData(unsigned int flags){}
	virtual const unsigned int * getExtents() const{return extents;}
	virtual unsigned int getDimension() const{return dims;}
        virtual unsigned int getTotalSize()const{return totalsize;}	
	virtual ~CPUStream();

  virtual int getFieldCount() const {return 1;}
  virtual __BRTStreamType getIndexedFieldType(int i) const {
    assert(i == 0);
    return elementType;
  }
    protected:
  __BRTStreamType elementType;
	void * data;
	unsigned int * extents;
	unsigned int dims;		
	unsigned int stride;
	unsigned int totalsize;

    };
   
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   class CPUIter:public Iter {
   protected:
    CPUStream stream;
    virtual ~CPUIter() {}
    void allocateStream(int dims, 
                        int extents[],
                        float ranges[]);
   public:
    CPUIter(__BRTStreamType type, int dims, int extents[], float ranges[])
    :Iter(type),stream(type,dims,extents){
      allocateStream(dims,extents,ranges);//now we always have this
    }
    virtual void * getData (unsigned int flags){return stream.getData(flags);}
    virtual void releaseData(unsigned int flags){stream.releaseData(flags);}
    virtual const unsigned int* getExtents()const {return stream.getExtents();}
    virtual unsigned int getDimension()const {return stream.getDimension();}
    //virtual __BRTStreamType getStreamType()const{return stream.getStreamType();}
    virtual unsigned int getTotalSize() const {return stream.getTotalSize();}
   };

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   class CPURunTime: public brook::RunTime {
   public:
	CPURunTime();
	virtual Kernel * CreateKernel(const void*[]);
	virtual Stream * CreateStream(
    int fieldCount, const __BRTStreamType fieldTypes[],
    int dims, const int extents[]);
	virtual Iter * CreateIter(__BRTStreamType type, 
                                  int dims, 
                                  int extents[],
                                  float ranges[]);
	virtual ~CPURunTime(){}
   };
}
#endif





