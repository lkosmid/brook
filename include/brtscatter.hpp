#ifndef BRTSCATTER_HPP
#define BRTSCATTER_HPP
#include "brtvector.hpp"
#include "brtarray.hpp"
#include "brtscatterintrinsic.hpp"
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

template <class T, class U, class V, class Functor> 
void scatterOpArrayHelper (const T* s, 
                      V *index,
                      U &out,
                      unsigned int size,
                      const Functor & op) {
   for (unsigned int i=0;i<size;++i) {
      op(out[index[i]],s[i]);
   }
}

template <class Functor, class T> 
void streamScatterOpIndexDet(const T* data,
                             const __BRTStream & index,
                             const __BRTStream & out,
                             unsigned int bounds,
                             const Functor &op) {
   unsigned int dim = out->getDimension();
   switch (index->getStreamType()) {
   case __BRTFLOAT4:
      if (dim!=4)dim=1;
      break;
   case __BRTFLOAT3:
      if (dim!=3)dim=1;
      break;
   case __BRTFLOAT2:
      if (dim!=2)dim=1;
      break;
   default:
      dim=1;
   }
   switch (dim) {
   case 4:{
      __BrtArray <T,4,false> o((T*)out->getData(brook::Stream::WRITE),
                               out->getExtents());
      scatterOpArrayHelper(data,
                           (__BrtFloat4 *)index->getData(brook::Stream::READ),
                           o,
                           bounds,
                           op);      
      break;
   }
   case 3:{
      __BrtArray<T,3,false> o((T*)out->getData(brook::Stream::WRITE),
                              out->getExtents());
      scatterOpArrayHelper(data,
                           (__BrtFloat3 *)index->getData(brook::Stream::READ),
                           o,
                           bounds,
                           op);      
      
      break;
   }
   case 2:{
      __BrtArray<T,2,false> o((T*)out->getData(brook::Stream::WRITE),
                              out->getExtents());
      scatterOpArrayHelper(data,
                      (__BrtFloat2 *)index->getData(brook::Stream::READ),
                      o,
                      bounds,
                      op);      
      break;
   }
   default:
      scatterOpHelper(data,
                      (float *)index->getData(brook::Stream::READ),
                      (T*)out->getData(brook::Stream::WRITE),
                      bounds,
                      op);
   }
}

#define streamScatterOp1 streamScatterOp
#define streamScatterOp2 streamScatterOp
#define streamScatterOp3 streamScatterOp
#define streamScatterOp4 streamScatterOp

                            
template <class Functor> void streamScatterOp (const __BRTStream &s, 
                                               const __BRTStream &index,
                                               const __BRTStream &array, 
                                               const Functor&op) {
   unsigned int bounds = s->getTotalSize();
   switch (s->getStreamType()) {
   case __BRTFLOAT4:{
      const __BrtFloat4* data = 
         (const __BrtFloat4 *) s->getData(brook::Stream::READ);
      streamScatterOpIndexDet(data,index,array,bounds,op);
      break;
   }
   case __BRTFLOAT3:{
      const __BrtFloat3* data = 
         (const __BrtFloat3 *) s->getData(brook::Stream::READ);
      streamScatterOpIndexDet(data,index,array,bounds,op);
      break;
   }
   case __BRTFLOAT2:{
      const __BrtFloat2* data = 
         (const __BrtFloat2 *) s->getData(brook::Stream::READ);
      streamScatterOpIndexDet(data,index,array,bounds,op);
      break;
   }
   case __BRTFLOAT:
   default:{
      const __BrtFloat1* data = 
         (const __BrtFloat1 *) s->getData(brook::Stream::READ);
      streamScatterOpIndexDet(data,index,array,bounds,op);
   }
      break;
   }
   s->releaseData(brook::Stream::READ); 
   array->releaseData(brook::Stream::READ);      
   index->releaseData(brook::Stream::READ);
}
#endif
