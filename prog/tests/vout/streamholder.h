#include "brt.hpp"
struct StreamHolder {
   __BRTStream * s;
   StreamHolder (__BRTStream * s){
      this->s = s;
   }
   StreamHolder (){
      this->s = 0;
   }
   __BRTStream * operator -> () {
      return s;
   }
   const __BRTStream * operator -> () const{
      return s;
   }
   operator brook::stream& () {
      return *s;
   }
   operator brook::stream *() {
      return s;
   }
   operator const brook::StreamInterface *() {
      return (brook::StreamInterface*)s;
   }
   __BRTStream & operator * () {
      return *s;
   }
   const __BRTStream & operator * () const{
      return *s;
   }   
};
inline StreamHolder * MakeStreamHolder(__BRTStream * s) {
   return reinterpret_cast<StreamHolder*>(s);
}







