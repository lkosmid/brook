class __StreamScatterAssign {
   template <class T> void operator () (T& out, const T& in)const{ 
      out=in;
   }
};
class __StreamScatterAdd {
   template <class T> T operator () (T& out, const T& in)const{ 
      return out+=in
   }
};
class __StreamScatterMul {
   template <class T> T operator () (T& out, const T& in)const{ 
      return out*=in
   }
}


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
      op(out[index[i]],s[i]);
   }
}
template <class Functor> void streamScatterOp (__BRTStream s, 
                                               __BRTStream index,
                                               __BRTStream array, 
                                               const Functor&op) {
   unsigned int bounds = s->getTotalSize();
   float * i = index->getData(brook::READ);
   switch (s->getStreamType()) {
   case __BRTFLOAT4:{
      const __BRTFloat4* data = (const __BRTFloat4 *) s->getData(brook::READ);
      __BRTFloat4* out = (__BRTFloat4 *) array->getData(brook::WRITE);
      scatterOpHelper(data,i,out,bounds,op);
      break;
   }
   case __BRTFLOAT3:{
      const __BRTFloat4* data = (const __BRTFloat3 *) s->getData(brook::READ);
      __BRTFloat4* out = (__BRTFloat3 *) array->getData(brook::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
      break;
   }
   case __BRTFLOAT2:{
      const __BRTFloat4* data = (const __BRTFloat2 *) s->getData(brook::READ);
      __BRTFloat4* out = (__BRTFloat2 *) array->getData(brook::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
      break;
   }
   case __BRTFLOAT:
   default:{
      const __BRTFloat4* data = (const __BRTFloat1 *) s->getData(brook::READ);
      __BRTFloat4* out = (__BRTFloat1 *) array->getData(brook::WRITE);
      scatterOpHelper(data,i,out,bounds,op);      
   }
      break;
   }
   s->releaseData(brook::READ);
   array->releaseData(brook::READ);      
   index->releaseData(brook::READ);
}
