#ifndef HANDVECTOR_HPP
#define HANDVECTOR_HPP
#if 0
//the template one is still faster than my hand coded one!!!
//change #if 0 to #if 1
#include "brtvector.hpp"
typedef __BrtFloat1 Float1;
#else
#include <iostream>
#include <math.h>
#include "type_promotion.hpp"

class Float1;
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
#if defined (_MSC_VER)
template <class T> class Holder {
public:
    static typename GetValueOf<T>::type getAt (const T&t, int i) {
        return t.getAt(i);
    }
};
#define HOLDER(TYP) template <> class Holder<TYP> { \
public: \
    static TYP getAt(TYP t, int i) { \
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
    return Holder<T>::getAt(in,i);
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

class Float1{
public:
#define tsize 1
    typedef float VALUE;
    typedef VALUE TYPE;
    enum SIZ{size=tsize};
    typedef VALUE array_type[size];
protected:
    VALUE f;
public:
    const VALUE &getAt (unsigned int i) const{
       return f;
    }
    VALUE &getAt (unsigned int i) {
       return f;
    }
    const VALUE &unsafeGetAt (unsigned int i) const{return f;}
    VALUE &unsafeGetAt (unsigned int i) {return f;}
    Float1 operator [] (int i)const {return BracketOp<VALUE>()(*this,i);}
    Float1& cast() {
        return *this;
    }
    const Float1& cast() const{
        return *this;
    }
    template<class BRT_TYPE> BRT_TYPE castTo() {
        return InitializeClass<BRT_TYPE>()(getAt(0),
					   getAt(1),
					   getAt(2),
					   getAt(3));
    }
#define BROOK_UNARY_OP(op) Float1 operator op ()const { \
      return Float1 (op getAt(0)); \
    }
    BROOK_UNARY_OP(+)
    BROOK_UNARY_OP(-)
    BROOK_UNARY_OP(!)    
#undef BROOK_UNARY_OP
    Float1 swizzle1(int x)const {
        return Float1(unsafeGetAt(x));
    }
    Float1() {}
#define GENERAL_TEMPLATIZED_FUNCTIONS
    template <class BRT_TYPE> 
      Float1 (const BRT_TYPE &inx, 
	   const BRT_TYPE &iny, 
	   const BRT_TYPE &inz, 
	   const BRT_TYPE& inw) {
        f=inx;
    }
    template <class BRT_TYPE> Float1 (const BRT_TYPE& inx, 
				   const BRT_TYPE& iny, 
				   const BRT_TYPE& inz) {
        f=inx;
    }
    template <class BRT_TYPE> Float1 (const BRT_TYPE& inx, const BRT_TYPE& iny) {
        f=inx;
    }
    Float1 (const float& scalar) {
        (*this)=scalar;
    }
    template <class BRT_TYPE> operator BRT_TYPE () const{
      return InitializeClass<BRT_TYPE>()(getAt(0),getAt(1),getAt(2),getAt(3));
    }
    Float1& operator = (const float & in) { 
        f = in;
        return *this;
    }
    Float1& operator = (const unsigned int & in) { 
        f = in;
        return *this;
    }
    Float1& operator = (const int & in) { 
        f = in;
        return *this;
    }
        
#define ASSIGN_OP(op) template <class BRT_TYPE> \
         Float1& operator op (const BRT_TYPE & in) {  \
        f op GetAt<BRT_TYPE>(in,0);  \
        return *this;  \
    }
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
        f=in.getAt(0);
        return vec<VALUE,4>(unsafeGetAt(X),
                            unsafeGetAt(Y),
                            unsafeGetAt(Z),
                            unsafeGetAt(W));
    }
    template <class BRT_TYPE>
      vec<VALUE,3> mask3 (const BRT_TYPE&in,int X,int Y,int Z) {
        f=in.getAt(0);
        return vec<VALUE,3>(unsafeGetAt(X),unsafeGetAt(Y),unsafeGetAt(Z));
    }
    template <class BRT_TYPE> 
      vec<VALUE,2> mask2 (const BRT_TYPE&in,int X,int Y) {
        f=in.getAt(0);
        return vec<VALUE,2>(unsafeGetAt(X),unsafeGetAt(Y));
    }
    template <class BRT_TYPE> 
      vec<VALUE,1> mask1 (const BRT_TYPE&in,int X) {
        f=in.getAt(0);
        return vec<VALUE,1>(unsafeGetAt(X));
    }    
    template <class BRT_TYPE> 
      Float1 questioncolon(const BRT_TYPE &b, 
						const BRT_TYPE &c)const {
       return f?b.getAt(0):c.getAt(0);
    }
#if defined (_MSC_VER) && (_MSC_VER <= 1200)
#define TEMPL_TYPESIZE sizeof(BRT_TYPE)/sizeof(BRT_TYPE::TYPE)
#else
#define TEMPL_TYPESIZE BRT_TYPE::size
#endif
#define BROOK_BINARY_OP(op,opgets,TYPESPECIFIER) template <class BRT_TYPE>          \
    Float1 operator op (const BRT_TYPE &b)const{ \
      return Float1(*this) opgets b; \
    }
    BROOK_BINARY_OP(*,*=,LCM);
    BROOK_BINARY_OP(/,/=,LCM);
    BROOK_BINARY_OP(+,+=,LCM);
    BROOK_BINARY_OP(-,-=,LCM);
    BROOK_BINARY_OP(%,%=,LCM);
#undef BROOK_BINARY_OP
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class BRT_TYPE>          \
    Float1 operator op (const BRT_TYPE &b)const{ \
      return Float1 \
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
#undef tsize
};



template <class T> 
  std::ostream& operator^ (std::ostream& os, const Float1 & a){
  os << a.getAt(0);
  return os;
}
template <class T> 
std::ostream& operator<< (std::ostream& os, const Float1 & a){
  os << a.getAt(0);
  return os;
}
#endif
#endif
