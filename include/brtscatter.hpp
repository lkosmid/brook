#ifndef BRTSCATTER_HPP
#define BRTSCATTER_HPP
#include "brtvector.hpp"
class __StreamScatterAssign {public:
   template <class T> void operator () (T& out, const T& in)const{ 
      out=in;
   }
};
class __StreamScatterAdd {public:
   template <class T> void operator () (T& out, const T& in)const{ 
      out+=in;
   }
};
class __StreamScatterMul {public:
   template <class T> void operator () (T& out, const T& in)const{ 
      out*=in;
   }
};


static __StreamScatterAssign STREAM_SCATTER_ASSIGN;
static __StreamScatterAdd STREAM_SCATTER_ADD;
static __StreamScatterMul STREAM_SCATTER_MUL;
#define STREAM_SCATTER_FLOAT_MUL STREAM_SCATTER_MUL
#define STREAM_SCATTER_FLOAT_ADD STREAM_SCATTER_ADD
#define STREAM_SCATTER_INTEGER_MUL STREAM_SCATTER_MUL
#define STREAM_SCATTER_INTEGER_ADD STREAM_SCATTER_ADD
template <class T, class Functor> void scatterOpHelper (const T* s, 
                                                        float *index,
                                                        T* out,
                                                        unsigned int size,
                                                        const Functor & op) {
   for (unsigned int i=0;i<size;++i) {
      op(out[(int)index[i]],s[i]);
   }
}
template <class Functor> void streamScatterOp (const __BRTStream &s, 
                                               const __BRTStream &index,
                                               const __BRTStream &array, 
                                               const Functor&op) {
   unsigned int bounds = s->getTotalSize();
   float * i = (float*)index->getData(brook::Stream::READ);
   switch (s->getStreamType()) {
   case __BRTFLOAT4:{
      const __BrtFloat4* data = 
         (const __BrtFloat4 *) s->getData(brook::Stream::READ);
      __BrtFloat4* out = 
         (__BrtFloat4 *) array->getData(brook::Stream::WRITE);
      scatterOpHelper(data,i,out,bounds,op);
      break;
   }
   case __BRTFLOAT3:{
      const __BrtFloat3* data = 
         (const __BrtFloat3 *) s->getData(brook::Stream::READ);
      __BrtFloat3* out = 
         (__BrtFloat3 *) array->getData(brook::Stream::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
      break;
   }
   case __BRTFLOAT2:{
      const __BrtFloat2* data = 
         (const __BrtFloat2 *) s->getData(brook::Stream::READ);
      __BrtFloat2* out = 
         (__BrtFloat2 *) array->getData(brook::Stream::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
      break;
   }
   case __BRTFLOAT:
   default:{
      const __BrtFloat1* data = 
         (const __BrtFloat1 *) s->getData(brook::Stream::READ);
      __BrtFloat1* out = 
         (__BrtFloat1 *) array->getData(brook::Stream::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
   }
      break;
   }
   s->releaseData(brook::Stream::READ);
   array->releaseData(brook::Stream::READ);      
   index->releaseData(brook::Stream::READ);
}
#endif
