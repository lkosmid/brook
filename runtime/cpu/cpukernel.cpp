#define USE_THREASD
#include <iostream>
#include<assert.h>
#include "cpu.hpp"
#include <stdio.h>
#ifdef USE_THREADS
#ifdef _WIN32
#include <windows.h>
#endif
#endif

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
struct BrtThread {
#ifdef USE_THREADS
#ifdef _WIN32
  DWORD Id;
#else
  pthread_t Id;
#endif
#else
   long Id;
#endif
  bool active;
};

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
static unsigned int getNumThreads() {
   static char * env = getenv ("BRT_NUM_THREADS");
   if (!env)
      return 4;
   unsigned int ret = atoi(env);
   if (ret<2)
      ret=1;
   return ret;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// Cross platform version of pthreads
static void BrtCreateThread(void* (*func)(void *), void* arg, BrtThread & ret){
  bool oncpu=true;
  ret.active=false;
#ifdef USE_THREADS
#ifdef _WIN32
    oncpu= (CreateThread(NULL,0,func,arg,0,&ret.Id)==NULL)
#else
    oncpu= (pthread_create(&ret.Id,NULL,func,arg)!=0);
#endif
#endif
  if (oncpu) 
    (*func)(arg);
  else
    ret.active=true;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// Cross platform version of pthreads
static void  BrtJoinThread (const BrtThread &id) {
  if (!id.active)
    return;
#ifdef USE_THREADS
#ifdef _WIN32
  WaitForSingleObject(id.Id, INFINITE);
  //!= WAIT_OBJECT;    
#else
  void *ret;
  pthread_join (id.Id,&ret);
#endif
#endif
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// Default function if no function was specified
static void nothing (const std::vector<void*>&args,
                     const std::vector<const unsigned int *>&extents,
                     const std::vector<unsigned int> &dims,
                     unsigned int start,
                     unsigned int end){}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// Any reductions passed to a kernel must have the same extents.
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

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    CPUKernel::CPUKernel(const void * source []){
        const char ** src= (const char**)(source);
        combine=0;func=0;
        multiThread=true;
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
               if (!combine) {
                  multiThread=false;
               }
            }
            if (strcmp(src[i],"ndcpu")==0) {
               nDfunc=(nDcallable*)source[i+1];
            }
	}
        Cleanup();
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushIter(Iter * i) {
       PushStreamInterface(i);
       //assert same size as output
       assert (iteroutsize==0||iteroutsize==i->getTotalSize());
       iteroutsize=i->getTotalSize();
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushStreamInterface(StreamInterface * s) {
	unsigned int total_size=s->getTotalSize();
	if (totalsize==0){//this is necessary for reductions
          totalsize=total_size;//don't override output tho
          dim=s->getDimension();
          extent= s->getExtents();
        }
        PushGatherStreamInterface(s);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushStream(Stream *s){
       PushStreamInterface(s);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushConstant(const float &val){
        args.push_back(const_cast<float*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushConstant(const float2 &val){
        args.push_back(const_cast<float2*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushConstant(const float3 &val){
        args.push_back(const_cast<float3*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushConstant(const float4 &val){
        args.push_back(const_cast<float4*>(&val));
        dims.push_back(0);
        extents.push_back(0);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushGatherStreamInterface(StreamInterface * s) {
        args.push_back(s->getData(brook::Stream::READ));
        extents.push_back(s->getExtents());
        dims.push_back(s->getDimension());
        inputs.push_back(s);

    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushGatherStream(Stream *s){
       PushGatherStreamInterface(s);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushReduce(void * data, __BRTStreamType type) {
       if (type==__BRTSTREAM) {
         brook::Stream * stream = *(const ::brook::stream *)data;
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
          reductions.push_back(ReductionArg(args.size(),type,NULL));
          args.push_back(data);
          dims.push_back(0);
          extents.push_back(0);
       }

    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::PushOutput(Stream *s){
        args.push_back(s->getData(brook::Stream::WRITE));
        assert (iteroutsize==0||iteroutsize==s->getTotalSize());
	totalsize=s->getTotalSize();
        dim=s->getDimension();
        extent= s->getExtents();
        dims.push_back(dim);
        extents.push_back(extent);
        outputs.push_back(s);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // This function calls release on any streams that may be owned by
   // DirectX or so forth.
    void CPUKernel::Cleanup() {
        reductions.clear();
        args.clear();
        iteroutsize=0;
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

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   //subMap is guaranteed that all reductions are actual values stored in args.
   //subMap is in charge of parallelizing threads where necessary.
    void CPUKernel::subMap(unsigned int begin, unsigned int end){
       (*func)(args,
               extents,
               dims,
               begin,
               end);
    }
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // This version of submap does the same thing as submap except that it 
   // is passed a new'd subMapInput struct that it will destroy.
   // The function has this signature because the thread create will invoke it
    void* CPUKernel::staticSubMap (void * inp) {
       subMapInput * submap = (subMapInput*)inp;
       (*submap->thus->func)(*submap,
                             submap->thus->extents,
                             submap->thus->dims,
                             submap->mapbegin,
                             submap->mapend);
       delete submap;
       return 0;
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // this funciton calls maps to numThreads portions of the stream
   // in parallel.  The last thread runs on the host cpu.
   // Reduction arguments are changed for each of the threads and combined
   // within this function.
    void CPUKernel::ThreadMap(unsigned int numThreads) {
       unsigned int i;
       std::vector<ReductionArg>::iterator j;
       unsigned int cur=0;
       unsigned int step = totalsize/numThreads;
       unsigned int remainder = totalsize%numThreads;
       std::vector<BrtThread>threads;
       for (i=0;i<numThreads-1;++i)
         threads.push_back(BrtThread());
       BrtCreateThread(staticSubMap,
                       new subMapInput(this,args,cur,step),
                       threads[0]);
       cur+=step;
       std::vector<void *>reductionbackup;
       for (j=reductions.begin();
            j!=reductions.end();
            ++j) {
          reductionbackup.push_back(args[j->which]);          
          args[j->which]=malloc(numThreads*j->type*sizeof(float));

       }
       for (i=1;i<numThreads-1;++i) {
          unsigned int thisstep=step;
          if (i<remainder)
             thisstep++;//leap year
          BrtCreateThread(staticSubMap,
                          new subMapInput(this,args,cur,thisstep),
                          threads[i]);
          cur+=thisstep;
          for (j=reductions.begin();j!=reductions.end();++j) {
             args[j->which]=((char *)args[j->which])+(j->type*sizeof(float));
          }          
       }
       subMap(cur,totalsize-cur);
       for (i=0;i<threads.size();++i) {
         BrtJoinThread(threads[i]);
       }
       cur=0;
       for (i=0;i<reductions.size();++i) {
          char * end = (char *)args[reductions[i].which];

          end-=reductions[i].type*sizeof(float)*(numThreads-2);
          args.push_back(end);                         
          args[reductions[i].which]=reductionbackup[i];          
       }

       if (combine!=0) {
          const unsigned int delta= args.size()-reductions.size();
          for (i=0;i<numThreads-1;++i) {
             unsigned int thisstep=step;
             if (i<remainder)
                thisstep++;//leap year
             (*combine)(args,extents,dims,cur);
             cur+=thisstep;
             for (unsigned int k=delta;
                  k<args.size();
                  ++k) {
                args[k]=((char *)args[k]) 
                   + (reductions[k-delta].type*sizeof(float));
             }     
          }
          for (i=delta;i<args.size();++i) {
             free( (char *)args[i]-reductions[i-delta].type*sizeof(float)*(numThreads-1));
          }
       }
    }
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // This function is invoked within a created thread to call the
   // nDfunc for a particular set of end values of the reduction stream
   // The values in the reduction stream start at cur and end before curfinal
   // for each reduction stream the tight nDfunc loop is called. The myargs
   // is expected to contain all values aside from the target reduction
   void CPUKernel::ReduceToStream (std::vector<void *>&myargs,
                                   unsigned int cur, 
                                   unsigned int curfinal,
                                   const unsigned int *extent,
                                   unsigned int rdim,
                                   unsigned int *mapbegin,
                                   const unsigned int *mag)const{
      std::vector<ReductionArg>::const_iterator j;
      for (j=reductions.begin();j!=reductions.end();++j) {
         myargs[(*j).which]=(char*)myargs[(*j).which]+
            cur*(*j).stream->getElementSize();
      }
      for (;cur<curfinal;++cur) {
         (*nDfunc)(myargs,extents,dims,mapbegin,mag);
         for (j=reductions.begin();j!=reductions.end();++j) {
            myargs[(*j).which]=(char*)myargs[(*j).which]+
               (*j).stream->getElementSize();
         }
         
         mapbegin[rdim-1]+=mag[rdim-1];
         unsigned int k;
         for (k=rdim-1;k>=1;--k) {
            if (mapbegin[k]>=extent[k]){
               mapbegin[k]=0;
               mapbegin[k-1]+=mag[k-1];                   
            }else break;
         }
      }
   }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   ///This function duplicates the input args and then calls ReduceToStream
   void * CPUKernel::staticReduceToStream(void * inp) {
      reduceToStreamInput * red = (reduceToStreamInput*)inp;
      red->thus->ReduceToStream(*red,
                                red->cur,
                                red->curfinal,
                                red->extent,
                                red->rdim,
                                red->mapbegin,
                                red->mag);
      delete red;
      return 0;
   }
   
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // This function calculates the beginning of the nDcube given a 
   // location on the resulting reduction stream.
   static void calcLocation(unsigned int * rez, 
                           unsigned int cur, 
                           unsigned int dim,
                           const unsigned int *mag, 
                           const unsigned int * extent) {
      for (int i=dim-1;i>=0;--i) {
         unsigned int sizei=extent[i]/mag[i];
         rez[i]=(cur%sizei)*mag[i];
         cur/=sizei;
      }
   }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
   // This function performs the map or reduction on the input arguments.
   // The reduction can be either multi or single threaded depending on the 
   // value of the combiner. if the combiner is explicitly set to null then 
   // the multiThread class var is false and this function goes through the
   // whole stream at once. Otherwise it splits up the work.
   // In a stream to stream reduction it calls ReduceToStream to work on
   // all or part of the stream. Otherwise it uses subMap or staticSubMap.
    void CPUKernel::Map() {
      unsigned int numThreads=getNumThreads();
       if (!streamReduction){
          if (multiThread&&totalsize/2) {             
             while (totalsize/numThreads==0)
                numThreads/=2;
             ThreadMap(numThreads);             
          }else {
             subMap(0,totalsize);
          }
       }else {
          unsigned int i;
          unsigned int rdim = streamReduction->getDimension();
          const unsigned int * rextent = streamReduction->getExtents();
          unsigned int total = streamReduction->getTotalSize();
          std::vector<ReductionArg>::iterator j;
          if (multiThread&&total) {
            while (total/numThreads==0)
              numThreads/=2;
          }else numThreads=1;
          unsigned int step=total/numThreads;
          unsigned int remainder = total%numThreads;
          
          for (j=reductions.begin();j!=reductions.end();++j) {
             assert ((*j).type==__BRTSTREAM);
             args[(*j).which]=(*j).stream->getData(brook::Stream::WRITE);
          }
          
          unsigned int * buffer = (unsigned int *)
             malloc(2*numThreads*dim*sizeof(unsigned int));
          memset (buffer,0,sizeof(unsigned int)*(1+numThreads)*dim);
          unsigned int * mag = buffer;
          unsigned int * e =mag+dim;
          unsigned int dimoffset = dim-rdim;
          for (i=0;i<dim;++i) {
             if (i>=dimoffset)
                mag[i] = extent[i]/rextent[i-dimoffset];
             else
                mag[i] = extent[i];             
          }
          unsigned int cur=step;
          if (remainder) cur++;
          for (i=1;i<numThreads;++i) {
            unsigned int *loc = e+dim*i;
            calcLocation(loc,cur,dim,mag,extent);
            cur+=step;
            if (i<remainder) {
              cur++;
            }
          }
          cur=0;
          std::vector<BrtThread> pthreads;
          for (i=0;i<numThreads-1;++i) {
              pthreads.push_back(BrtThread());
          }
          for (unsigned int threads=0;threads<numThreads;++threads) {
            unsigned int curfinal=cur+step;
            if (threads<remainder) curfinal++;
            unsigned int * mapbegin=e+threads*dim;
            if (threads!=numThreads-1) {
              BrtCreateThread(                             
                             CPUKernel::staticReduceToStream,
                             new reduceToStreamInput(this,
                                                     args,
                                                     cur,
                                                     curfinal,
                                                     extent,
                                                     rdim,
                                                     mapbegin,
                                                     mag),
                             pthreads.back());
            }else{
              ReduceToStream(args,cur,curfinal,extent,rdim,mapbegin,mag);
            }
            
            cur=curfinal;
          }
          for (i=0;i<pthreads.size();++i) {
            BrtJoinThread (pthreads[i]);
          }
          free (buffer);
          for (j=reductions.begin();j!=reductions.end();++j) {
             (*j).stream->releaseData(brook::Stream::WRITE);
          }
       }
       Cleanup();
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUKernel::Release() {
	delete this;
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    CPUKernel::~CPUKernel() {

    }
}


