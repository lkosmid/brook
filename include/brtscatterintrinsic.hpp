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

extern  __StreamScatterAssign STREAM_SCATTER_ASSIGN;
extern  __StreamScatterAdd STREAM_SCATTER_ADD;
extern  __StreamScatterMul STREAM_SCATTER_MUL;
