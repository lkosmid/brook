#ifndef BRTVECTOR_HPP
#define BRTVECTOR_HPP
#if defined (_MSC_VER) && _MSC_VER <=1200 && !defined(VC6VECTOR_HPP)
#include "vc6vector.hpp"
//the above headerfile has the template functions automatically expanded.
//not needed for recent compilers.
#else

#include <iostream>
#include <math.h>
template <class T, class B> static T singlequestioncolon (const B& a,
                                                          const T&b,
                                                          const T&c){
    return a.questioncolon(b,c);
};
template <> static float singlequestioncolon (const char & a,
                                              const float &b,
                                              const float&c) {
    return a?b:c;
}
template <> static float singlequestioncolon (const float & a,
                                              const float &b,
                                              const float&c) {
    return a?b:c;
}

template <unsigned int sizeA, unsigned int sizeB> class LUB {
public:
    enum SIZ{size=((sizeA==0&&sizeB==0)?1:
		   (sizeA==1)?sizeB:(sizeB==1)?sizeA:
		   (sizeA>sizeB)?sizeB:sizeA)
    };
};
template<> class LUB<1,1> {public:
    enum SIZ{size=1};
};
template<> class LUB<1,2> {public:
    enum SIZ{size=2};
};
template<> class LUB<2,1> {public:
    enum SIZ{size=2};
};
template<> class LUB<1,3> {public:
    enum SIZ{size=3};
};
template<> class LUB<3,1> {public:
    enum SIZ{size=3};
};
template<> class LUB<1,4> {public:
    enum SIZ{size=4};
};
template<> class LUB<4,1> {public:
    enum SIZ{size=4};
};
template<> class LUB<2,2> {public:
    enum SIZ{size=2};
};
template<> class LUB<2,3> {public:
    enum SIZ{size=2};
};
template<> class LUB<3,2> {public:
    enum SIZ{size=2};
};
template<> class LUB<2,4> {public:
    enum SIZ{size=2};
};
template<> class LUB<4,2> {public:
    enum SIZ{size=2};
};
template<> class LUB<3,3> {public:
    enum SIZ{size=3};
};
template<> class LUB<3,4> {public:
    enum SIZ{size=3};
};
template<> class LUB<4,3> {public:
    enum SIZ{size=3};
};
template<> class LUB<4,4> {public:
    enum SIZ{size=4};
};

template <class VALUE, unsigned int tsize> class vec;

template <class A, class B> class LCM {
public:
    typedef vec<typename LCM<typename A::TYPE , typename B::TYPE>::type,
                LUB<A::size,B::size>::size> type;
};
template <> class LCM<int,int> {public:
    typedef int type;
};
template <> class LCM<int,float> {public:
    typedef float type;
};
template <> class LCM<float,int> {public:
    typedef float type;
};
template <> class LCM<float,float> {public:
    typedef float type;
};
template <> class LCM<char,int> {public:
    typedef int type;
};
template <> class LCM<int,char> {public:
    typedef int type;
};
template <> class LCM<char,char> {public:
    typedef char type;
};
template <> class LCM<float,char> {public:
    typedef float type;
};
template <> class LCM<char,float> {public:
    typedef float type;
};


template <> class LCM<vec<int,1>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<int,1> > {public:
    typedef int type;
};
template <> class LCM<vec<int,1>,float> {public:
    typedef float type;
};
template <> class LCM<int,vec<float,1> > {public:
    typedef float type;
};
template <> class LCM<float,vec<int,1> > {public:
    typedef float type;
};
template <> class LCM<vec<float,1>,int > {public:
    typedef float type;
};
template <> class LCM<vec<float,1>,float> {public:
    typedef float type;
};
template <> class LCM<float,vec<float,1> > {public:
    typedef float type;
};

template <> class LCM<char,vec<int,1> > {public:
    typedef int type;
};
template <> class LCM<vec<char,1>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<char,1> > {public:
    typedef int type;
};
template <> class LCM<vec<int,1>,char> {public:
    typedef int type;
};
template <> class LCM<char,vec<char,1> > {public:
    typedef char type;
};
template <> class LCM<vec<char,1>,char> {public:
    typedef char type;
};
template <> class LCM<float,vec<char,1> > {public:
    typedef float type;
};
template <> class LCM<vec<float,1>,char> {public:
    typedef float type;
};
template <> class LCM<vec<char,1>,float> {public:
    typedef float type;
};
template <> class LCM<char,vec<float,1> > {public:
    typedef float type;
};




template <class A, class B> class COMMON_CHAR {
public:
   typedef vec<typename COMMON_CHAR<typename A::TYPE , 
                                    typename B::TYPE>::type,
               LUB<A::size,B::size>::size> type;
};
template <> class COMMON_CHAR<int,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,float> {public:
    typedef char type;
};



template <> class COMMON_CHAR<vec<int,1>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<int,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,1>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<float,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<int,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,1>,int > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,1>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<float,1> > {public:
    typedef char type;
};

template <> class COMMON_CHAR<char,vec<int,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,1>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<char,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,1>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<char,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,1>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<char,1> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,1>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,1>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<float,1> > {public:
    typedef char type;
};


template <class T> class GetValueOf {public:
    typedef typename T::TYPE type;
};
template <> class GetValueOf <float> {public:
    typedef float type;
};
template <> class GetValueOf <double> {public:
    typedef double type;
};
template <> class GetValueOf <int> {public:
    typedef int type;
};
template <> class GetValueOf <char> {public:
    typedef char type;
};

template <class T> class Holder {
public:
    const T&t;
    Holder(const T&tt) : t(tt) {}
    inline typename GetValueOf<T>::type getAt (int i) {
        return t.getAt(i);
    }
};
template <> class Holder<float> {
public:
    float t;
    Holder(float tt):t(tt) {}
    inline float getAt(int i) {
        return t;
    }
};
template <> class Holder<double> {
public:
    double t;
    Holder(double tt):t(tt) {}
    inline double getAt(int i) {
        return t;
    }
};
template <> class Holder<char> {
public:
    char t;
    Holder(char tt):t(tt) {}
    inline char getAt(int i) {
        return t;
    }
};
template <> class Holder<int> {
public:
    int t;
    Holder(int tt):t(tt) {}
    inline int getAt(int i) {
        return t;
    }
};

template <class T> class BracketType {public:
  typedef T type;
};
template <> class BracketType <float> {public:
  typedef vec<float,1> type;
};
template <> class BracketType <int> {public:
  typedef vec<int,1> type;
};
template <> class BracketType <char> {public:
  typedef vec<char,1> type;
};

template <class T> class BracketOp {public:
  template <class U> T& operator ()(const U&u, unsigned int i) {
    return u.getAt(i);
  }
  template <class U> T& operator () (U&u, unsigned int i) {
    return u.getAt(i);
  }
};
template <> class BracketOp <float> {public:
  template <class U> U& operator ()(const U&u, unsigned int i) {return u;}
  template <class U> U& operator () (U&u, unsigned int i) {return u;}
};
template <> class BracketOp <int> {public:
  template <class U> U& operator ()(const U&u, unsigned int i) {return u;}
  template <class U> U& operator () (U&u, unsigned int i) {return u;}
};
template <> class BracketOp <char> {public:
  template <class U> U& operator ()(const U&u, unsigned int i) {return u;}
  template <class U> U& operator () (U&u, unsigned int i) {return u;}
};



template <class T> typename GetValueOf<T>::type GetAt (const T& in,int i) {
    return Holder<T>(in).getAt(i);
}
enum MASKS {
  maskX=0,
  maskY=1,
  maskZ=2,
  maskW=3
};
template <class T> class InitializeClass {public:
    template <class V> T operator () (const V&a, const V&b, const V&c,
const V&d) {
        return T(a,b,c,d);
    }
};
#define INITBASECLASS(MYTYPE) template <> class InitializeClass<MYTYPE> { \
 public: \
    template <class V> MYTYPE operator () (const V&a, \
					  const V&b,  \
					  const V&c,  \
					  const V&d) { \
      return (MYTYPE)a; \
    } \
}
INITBASECLASS(float);
INITBASECLASS(double);
INITBASECLASS(int);
INITBASECLASS(unsigned int);
INITBASECLASS(bool);
INITBASECLASS(char);
INITBASECLASS(unsigned char);

#ifdef _MSC_VER
#if _MSC_VER <= 1200
#define GCCTYPENAME
#define INTERNALTYPENAME
#else
#define GCCTYPENAME typename
#define INTERNALTYPENAME typename
#endif
#else
#define GCCTYPENAME typename
#define INTERNALTYPENAME typename
#endif


//#include "vc6vector.hpp"

template <class VALUE, unsigned int tsize> class vec{
public:
    typedef VALUE TYPE;
    enum SIZ{size=tsize};
    typedef VALUE array_type[size];
protected:
    VALUE f[size];
public:
    const VALUE &getAt (unsigned int i) const{return f[i%size];}
    VALUE &getAt (unsigned int i) {return f[i%size];}
    const typename BracketType<VALUE>::type &operator [] (unsigned int i)const {return BracketOp<VALUE>()(*this,i);}
    typename BracketType<VALUE>::type &operator [] (unsigned int i) {return BracketOp<VALUE>()(*this,i);}
    vec<VALUE,tsize>& cast() {
        return *this;
    }
    const vec<VALUE,tsize>& cast() const{
        return *this;
    }
    template<class BRT_TYPE> BRT_TYPE castTo() {
        return InitializeClass<BRT_TYPE>()(getAt(0),
					   getAt(1),
					   getAt(2),
					   getAt(3));
    }
#define BROOK_UNARY_OP(op) vec<VALUE,tsize> operator op ()const { \
      return vec<VALUE, tsize > (op getAt(0),  \
                                 op getAt(1),  \
                                 op getAt(2),  \
                                 op getAt(3)); \
    }
    BROOK_UNARY_OP(+)
    BROOK_UNARY_OP(-)
    BROOK_UNARY_OP(!)    
#undef BROOK_UNARY_OP
    vec<VALUE,4> swizzle4(MASKS x,MASKS y,MASKS z,MASKS w)const {
        return vec<VALUE,4>(getAt(x),
                            getAt(y),
                            getAt(z),
                            getAt(w));
    }
    vec<VALUE,3> swizzle3(MASKS x,MASKS y,MASKS z)const {
        return vec<VALUE,3>(getAt(x),getAt(y),getAt(z));
    }
    vec<VALUE,2> swizzle2(MASKS x,MASKS y)const {
        return vec<VALUE,2>(getAt(x),getAt(y));
    }
    vec<VALUE, 1> swizzle1(MASKS x)const {
        return vec<VALUE,1>(getAt(x));
    }
    vec() {
      //for (unsigned int i=0;i<size;++i) 
      //      f[i]=VALUE();
    }
#define GENERAL_TEMPLATIZED_FUNCTIONS
    template <class BRT_TYPE> 
      vec (const BRT_TYPE &inx, 
	   const BRT_TYPE &iny, 
	   const BRT_TYPE &inz, 
	   const BRT_TYPE& inw) {
        f[0]=inx;
        if (size>1) f[1]=iny;
        if (size>2) f[2]=inz;
        if (size>3) f[3]=inw;
    }
    template <class BRT_TYPE> vec (const BRT_TYPE& inx, 
				   const BRT_TYPE& iny, 
				   const BRT_TYPE& inz) {
        f[0]=inx;if(size>1)f[1]=iny;if(size>2)f[2]=inz;if(size>3)f[3]=VALUE();
    }
    template <class BRT_TYPE> vec (const BRT_TYPE& inx, const BRT_TYPE& iny) {
        f[0]=inx;
        if (size>1) f[1]=iny;
        if (size>2) f[2]=VALUE();
        if (size>3) f[3]=VALUE();
    }
    template <class BRT_TYPE> vec (const BRT_TYPE& scalar) {
        (*this)=scalar;
    }
    template <class BRT_TYPE> operator BRT_TYPE () const{
      return InitializeClass<BRT_TYPE>()(getAt(0),getAt(1),getAt(2),getAt(3));
    }        
#define ASSIGN_OP(op) template <class BRT_TYPE> \
         vec<VALUE,tsize>& operator op (const BRT_TYPE & in) {  \
        f[0] op GetAt<BRT_TYPE>(in,0);  \
        if (tsize>1) f[1] op GetAt<BRT_TYPE>(in,1);  \
        if (tsize>2) f[2] op GetAt<BRT_TYPE>(in,2);  \
        if (tsize>3) f[3] op GetAt<BRT_TYPE>(in,3);  \
        return *this;  \
    }
    ASSIGN_OP(=);
    ASSIGN_OP(/=);
    ASSIGN_OP(+=);
    ASSIGN_OP(-=);
    ASSIGN_OP(*=);
    ASSIGN_OP(%=);
#undef ASSIGN_OP
#undef GENERAL_TEMPLATIZED_FUNCTIONS
#define VECTOR_TEMPLATIZED_FUNCTIONS
    template <class BRT_TYPE>
      vec<VALUE,4> mask4 (const BRT_TYPE&in,MASKS X, MASKS Y,MASKS Z,MASKS W) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        if (tsize>Z)f[Z]=in.getAt(2);
        if (tsize>W)f[W]=in.getAt(3);
        return vec<VALUE,4>(getAt(X),
                    getAt(Y),
                    getAt(Z),
                    getAt(W));
    }
    template <class BRT_TYPE>
      vec<VALUE,3> mask3 (const BRT_TYPE&in,MASKS X,MASKS Y,MASKS Z) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        if (tsize>Z)f[Z]=in.getAt(2);
        return vec<VALUE,3>(getAt(X),getAt(Y),getAt(Z));
    }
    template <class BRT_TYPE> 
      vec<VALUE,2> mask2 (const BRT_TYPE&in,MASKS X,MASKS Y) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        return vec<VALUE,2>(getAt(X),getAt(Y));
    }
    template <class BRT_TYPE> 
      vec<VALUE,1> mask1 (const BRT_TYPE&in,MASKS X) {
        if (tsize>X)f[X]=in.getAt(0);
        return vec<VALUE,1>(getAt(X));
    }    
    template <class BRT_TYPE> 
      vec<typename BRT_TYPE::TYPE,tsize> questioncolon(const BRT_TYPE &b, 
						const BRT_TYPE &c)const {
        return vec<GCCTYPENAME BRT_TYPE::TYPE,tsize>
            (singlequestioncolon(getAt(0),b.getAt(0),c.getAt(0)),
             singlequestioncolon(getAt(1),b.getAt(1),c.getAt(1)),
             singlequestioncolon(getAt(2),b.getAt(2),c.getAt(2)),
             singlequestioncolon(getAt(3),b.getAt(3),c.getAt(3)));
    }
#undef VECTOR_TEMPLATIZED_FUNCTIONS
#define OPERATOR_TEMPLATIZED_FUNCTIONS
#if defined (_MSC_VER) && (_MSC_VER <= 1200)
#define TEMPL_TYPESIZE sizeof(BRT_TYPE)/sizeof(BRT_TYPE::TYPE)
#else
#define TEMPL_TYPESIZE BRT_TYPE::size
#endif
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class BRT_TYPE>          \
    vec<GCCTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE,VALUE>::type, \
       LUB<TEMPL_TYPESIZE,tsize>::size> operator op (const BRT_TYPE &b)const{ \
      return vec<INTERNALTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE, \
                                           VALUE>::type, \
		 LUB<TEMPL_TYPESIZE,tsize>::size> \
                (getAt(0) op b.getAt(0), \
                 getAt(1) op b.getAt(1), \
                 getAt(2) op b.getAt(2), \
                 getAt(3) op b.getAt(3)); \
    }
    BROOK_BINARY_OP(*,LCM);
    BROOK_BINARY_OP(/,LCM);
    BROOK_BINARY_OP(+,LCM);
    BROOK_BINARY_OP(-,LCM);
    BROOK_BINARY_OP(%,LCM);
    BROOK_BINARY_OP(||,LCM);
    BROOK_BINARY_OP(&&,LCM);
    BROOK_BINARY_OP(<,COMMON_CHAR)
    BROOK_BINARY_OP(>,COMMON_CHAR)        
    BROOK_BINARY_OP(<=,COMMON_CHAR)
    BROOK_BINARY_OP(>=,COMMON_CHAR)        
    BROOK_BINARY_OP(!=,COMMON_CHAR)
    BROOK_BINARY_OP(==,COMMON_CHAR)
#undef TEMPL_TYPESIZE
#undef BROOK_BINARY_OP    
#undef OPERATOR_TEMPLATIZED_FUNCTIONS
};



template <class T> 
  std::ostream& operator^ (std::ostream& os, const T & a){
    if (T::size==1) {
        os << a.getAt(0);
    }else {
        os << "{";
        for (unsigned int i=0;i<T::size;++i) {
            os << a.getAt(i)<<(i!=T::size-1?", ":"");
        }
        os << "}";
    }
    return os;
}

#define VECX_CLASS(NAME,TYPE,X) \
inline std::ostream& operator << (std::ostream&a,const vec<TYPE,X> & b) { \
    return a^b; \
}   \
typedef vec<TYPE,X> NAME

VECX_CLASS(__BrtFloat0,float,1);
VECX_CLASS(__BrtFloat1,__BrtFloat0,1);
VECX_CLASS(__BrtFloat2,__BrtFloat0,2);
VECX_CLASS(__BrtFloat3,__BrtFloat0,3);
VECX_CLASS(__BrtFloat4,__BrtFloat0,4);
VECX_CLASS(__BrtChar0,char,1);
VECX_CLASS(__BrtChar1,__BrtChar0,1);
VECX_CLASS(__BrtChar2,__BrtChar0,2);
VECX_CLASS(__BrtChar3,__BrtChar0,3);
VECX_CLASS(__BrtChar4,__BrtChar0,4);
VECX_CLASS(__BrtInt0,int,1);
VECX_CLASS(__BrtInt1,__BrtInt0,1);
VECX_CLASS(__BrtInt2,__BrtInt0,2);
VECX_CLASS(__BrtInt3,__BrtInt0,3);
VECX_CLASS(__BrtInt4,__BrtInt0,4);
#undef VECX_CLASS
#define MATRIXXY_CLASS(TYPE,X,Y) \
inline std::ostream& operator << (std::ostream&a, \
                                  const vec<vec<TYPE##0,X>,Y> & b) { \
    return a^b; \
}   \
typedef vec<vec<TYPE##0,X>,Y> TYPE##X##x##Y

MATRIXXY_CLASS(__BrtFloat,4,4);
MATRIXXY_CLASS(__BrtFloat,4,3);
MATRIXXY_CLASS(__BrtFloat,4,2);
MATRIXXY_CLASS(__BrtFloat,4,1);
MATRIXXY_CLASS(__BrtFloat,3,4);
MATRIXXY_CLASS(__BrtFloat,3,3);
MATRIXXY_CLASS(__BrtFloat,3,2);
MATRIXXY_CLASS(__BrtFloat,3,1);
MATRIXXY_CLASS(__BrtFloat,2,4);
MATRIXXY_CLASS(__BrtFloat,2,3);
MATRIXXY_CLASS(__BrtFloat,2,2);
MATRIXXY_CLASS(__BrtFloat,2,1);
MATRIXXY_CLASS(__BrtFloat,1,4);
MATRIXXY_CLASS(__BrtFloat,1,3);
MATRIXXY_CLASS(__BrtFloat,1,2);
MATRIXXY_CLASS(__BrtFloat,1,1);

MATRIXXY_CLASS(__BrtInt,4,4);
MATRIXXY_CLASS(__BrtInt,4,3);
MATRIXXY_CLASS(__BrtInt,4,2);
MATRIXXY_CLASS(__BrtInt,4,1);
MATRIXXY_CLASS(__BrtInt,3,4);
MATRIXXY_CLASS(__BrtInt,3,3);
MATRIXXY_CLASS(__BrtInt,3,2);
MATRIXXY_CLASS(__BrtInt,3,1);
MATRIXXY_CLASS(__BrtInt,2,4);
MATRIXXY_CLASS(__BrtInt,2,3);
MATRIXXY_CLASS(__BrtInt,2,2);
MATRIXXY_CLASS(__BrtInt,2,1);
MATRIXXY_CLASS(__BrtInt,1,4);
MATRIXXY_CLASS(__BrtInt,1,3);
MATRIXXY_CLASS(__BrtInt,1,2);
MATRIXXY_CLASS(__BrtInt,1,1);

MATRIXXY_CLASS(__BrtChar,4,4);
MATRIXXY_CLASS(__BrtChar,4,3);
MATRIXXY_CLASS(__BrtChar,4,2);
MATRIXXY_CLASS(__BrtChar,4,1);
MATRIXXY_CLASS(__BrtChar,3,4);
MATRIXXY_CLASS(__BrtChar,3,3);
MATRIXXY_CLASS(__BrtChar,3,2);
MATRIXXY_CLASS(__BrtChar,3,1);
MATRIXXY_CLASS(__BrtChar,2,4);
MATRIXXY_CLASS(__BrtChar,2,3);
MATRIXXY_CLASS(__BrtChar,2,2);
MATRIXXY_CLASS(__BrtChar,2,1);
MATRIXXY_CLASS(__BrtChar,1,4);
MATRIXXY_CLASS(__BrtChar,1,3);
MATRIXXY_CLASS(__BrtChar,1,2);
MATRIXXY_CLASS(__BrtChar,1,1);

#undef MATRIXXY_CLASS
#endif
#endif
