#ifndef BRTVECTOR_HPP
#define BRTVECTOR_HPP
#if defined (_MSC_VER) && _MSC_VER <=1200 && !defined(VC6VECTOR_HPP)
#include "vc6vector.hpp"
//the above headerfile has the template functions automatically expanded.
//not needed for recent compilers.
#else

#include <iostream>
#include <math.h>
#include "type_promotion.hpp"
template <class VALUE, unsigned int tsize> class vec;

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
template <> class GetValueOf <unsigned int> {public:
    typedef unsigned int type;
};
template <> class GetValueOf <char> {public:
    typedef char type;
};
template <> class GetValueOf <bool> {public:
    typedef bool type;
};
#if defined (_MSC_VER) && _MSC_VER <=1200
template <class T> class Holder {
public:
    typename GetValueOf<T>::type getAt (const T&t, int i) {
        return t.getAt(i);
    }
};
#define HOLDER(TYP) template <> class Holder<TYP> { \
public: \
    inline TYP getAt(TYP t, int i) { \
        return t; \
    } \
}
HOLDER(float);
HOLDER(double);
HOLDER(char);
HOLDER(unsigned int);
HOLDER(int);
HOLDER(bool);
template <class T> typename GetValueOf<T>::type GetAt (const T& in,int i) {
    return Holder<T>().getAt(in,i);
}
#else
template <class T> static typename GetValueOf<T>::type GetAt (const T& in,int i) {
    return in.getAt(i);
}
#define SPECIALGETAT(TYP) template <> static TYP GetAt (const TYP& in,int i) {return in;}

SPECIALGETAT(int)
SPECIALGETAT(unsigned int)
SPECIALGETAT(char)
SPECIALGETAT(float)
SPECIALGETAT(double)
SPECIALGETAT(bool)

#endif
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
    return u.unsafeGetAt(i);
  }
  template <class U> T& operator () (U&u, unsigned int i) {
    return u.unsafeGetAt(i);
  }
};
template <> class BracketOp <float> {public:
  template <class U> U operator ()(const U&u, unsigned int i) {return u;}
};
template <> class BracketOp <int> {public:
  template <class U> U operator ()(const U&u, unsigned int i) {return u;}
};
template <> class BracketOp <char> {public:
  template <class U> U operator ()(const U&u, unsigned int i) {return u;}
};

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

template <class VALUE, unsigned int tsize> class vec{
public:
    typedef VALUE TYPE;
    enum SIZ{size=tsize};
    typedef VALUE array_type[size];
protected:
    VALUE f[size];
public:
    const VALUE &getAt (unsigned int i) const{
       return i<size?f[i]:f[size-1];
    }
    VALUE &getAt (unsigned int i) {
       return i<size?f[i]:f[size-1];
    }
    const VALUE &unsafeGetAt (unsigned int i) const{return f[i];}
    VALUE &unsafeGetAt (unsigned int i) {return f[i];}
    typename BracketType<VALUE>::type operator [] (int i)const {return BracketOp<VALUE>()(*this,i);}
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
    vec<VALUE,4> swizzle4(int x,int y,int z,int w)const {
        return vec<VALUE,4>(getAt(x),
                            getAt(y),
                            getAt(z),
                            getAt(w));
    }
    vec<VALUE,3> swizzle3(int x,int y,int z)const {
        return vec<VALUE,3>(getAt(x),getAt(y),getAt(z));
    }
    vec<VALUE,2> swizzle2(int x,int y)const {
        return vec<VALUE,2>(getAt(x),getAt(y));
    }
    vec<VALUE, 1> swizzle1(int x)const {
        return vec<VALUE,1>(getAt(x));
    }
    vec() {}
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
      vec<VALUE,4> mask4 (const BRT_TYPE&in,int X, int Y,int Z,int W) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        if (tsize>Z)f[Z]=in.getAt(2);
        if (tsize>W)f[W]=in.getAt(3);
        return vec<VALUE,4>(unsafeGetAt(X),
                    unsafeGetAt(Y),
                    unsafeGetAt(Z),
                    unsafeGetAt(W));
    }
    template <class BRT_TYPE>
      vec<VALUE,3> mask3 (const BRT_TYPE&in,int X,int Y,int Z) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        if (tsize>Z)f[Z]=in.getAt(2);
        return vec<VALUE,3>(unsafeGetAt(X),unsafeGetAt(Y),unsafeGetAt(Z));
    }
    template <class BRT_TYPE> 
      vec<VALUE,2> mask2 (const BRT_TYPE&in,int X,int Y) {
        if (tsize>X)f[X]=in.getAt(0);
        if (tsize>Y)f[Y]=in.getAt(1);
        return vec<VALUE,2>(unsafeGetAt(X),unsafeGetAt(Y));
    }
    template <class BRT_TYPE> 
      vec<VALUE,1> mask1 (const BRT_TYPE&in,int X) {
        if (tsize>X)f[X]=in.getAt(0);
        return vec<VALUE,1>(unsafeGetAt(X));
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
#if defined (_MSC_VER) && (_MSC_VER <= 1200)
#define TEMPL_TYPESIZE sizeof(BRT_TYPE)/sizeof(BRT_TYPE::TYPE)
#else
#define TEMPL_TYPESIZE BRT_TYPE::size
#endif
#define BROOK_BINARY_OP(op,opgets,TYPESPECIFIER) template <class BRT_TYPE>          \
    vec<GCCTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE,VALUE>::type, \
       LUB<TEMPL_TYPESIZE,tsize>::size> operator op (const BRT_TYPE &b)const{ \
      return vec<INTERNALTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE, \
                                                VALUE>::type, \
		 LUB<TEMPL_TYPESIZE,tsize>::size>() opgets b; \
    }
    BROOK_BINARY_OP(*,*=,LCM);
    BROOK_BINARY_OP(/,/=,LCM);
    BROOK_BINARY_OP(+,+=,LCM);
    BROOK_BINARY_OP(-,-=,LCM);
    BROOK_BINARY_OP(%,%=,LCM);
#undef BROOK_BINARY_OP
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class BRT_TYPE>          \
    vec<GCCTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE,VALUE>::type, \
       LUB<TEMPL_TYPESIZE,tsize>::size> operator op (const BRT_TYPE &b)const{ \
      return vec<INTERNALTYPENAME TYPESPECIFIER<GCCTYPENAME BRT_TYPE::TYPE, \
                                           VALUE>::type, \
		 LUB<TEMPL_TYPESIZE,tsize>::size> \
                (getAt(0) op GetAt<BRT_TYPE>(b,0), \
                 getAt(1) op GetAt<BRT_TYPE>(b,1), \
                 getAt(2) op GetAt<BRT_TYPE>(b,2), \
                 getAt(3) op GetAt<BRT_TYPE>(b,3)); \
    }
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
#undef VECTOR_TEMPLATIZED_FUNCTIONS
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

VECX_CLASS(__BrtFloat1,float,1);
VECX_CLASS(__BrtFloat2,float,2);
VECX_CLASS(__BrtFloat3,float,3);
VECX_CLASS(__BrtFloat4,float,4);
VECX_CLASS(__BrtChar1,char,1);
VECX_CLASS(__BrtChar2,char,2);
VECX_CLASS(__BrtChar3,char,3);
VECX_CLASS(__BrtChar4,char,4);
VECX_CLASS(__BrtInt1,int,1);
VECX_CLASS(__BrtInt2,int,2);
VECX_CLASS(__BrtInt3,int,3);
VECX_CLASS(__BrtInt4,int,4);
#undef VECX_CLASS
#define MATRIXXY_CLASS(TYPE,X,Y) \
inline std::ostream& operator << (std::ostream&a, \
                                  const vec<TYPE##X,Y> & b) { \
    return a^b; \
}   \
typedef vec<TYPE##X,Y> TYPE##X##x##Y

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
