#include <iostream>
#include <math.h>
#define GCCTYPENAME typename
template <class VALUE, unsigned int tsize> class vec{
public:
    typedef VALUE TYPE;
    enum SIZ{size=tsize};
protected:
    VALUE f[size];
public:
    template <class T> operator T () const{
      InitializeClass<T> a; 
      return a(getAt(0),getAt(1),getAt(2),getAt(3));
    }        
    const VALUE &getAt (unsigned int i) const{return f[i%size];}
    const VALUE &operator [] (unsigned int i)const {return f[i%size];}
    VALUE &operator [] (unsigned int i) {return f[i%size];}
    vec<VALUE,tsize>& cast() {
        return *this;
    }
    const vec<VALUE,tsize>& cast() const{
        return *this;
    }
    template<class T> T castTo() {
        InitializeClass<T> a; 
        return a(getAt(0),getAt(1),getAt(2),getAt(3));
    }
    template<class x, class y, class z, class w> 
      vec<VALUE,4> swizzle4(x,y,z,w)const {
        return vec<VALUE,4>(getAt(x::ref),
                            getAt(y::ref),
                            getAt(z::ref),
                            getAt(w::ref));
    }
    template<class x, class y, class z> vec<VALUE,3> swizzle3(x,y,z)const {
        return vec<VALUE,3>(getAt(x::ref),getAt(y::ref),getAt(z::ref));
    }
    template<class x, class y> vec<VALUE,2> swizzle2(x,y)const {
        return vec<VALUE,2>(getAt(x::ref),getAt(y::ref));
    }
    template<class x> vec<VALUE, 1> swizzle1(x)const {
        return vec<VALUE,1>(getAt(x::ref));
    }
#define ASSIGN_OP(op) template <class T> \
         vec<VALUE,tsize>& operator op (const T & in) {  \
        f[0] op in.getAt(0);  \
        if (size>1) f[1] op in.getAt(1);  \
        if (size>2) f[2] op in.getAt(2);  \
        if (size>3) f[3] op in.getAt(3);  \
        return *this;  \
    }
    ASSIGN_OP(=);
    ASSIGN_OP(/=);
    ASSIGN_OP(+=);
    ASSIGN_OP(-=);
    ASSIGN_OP(*=);
    ASSIGN_OP(%=);
#undef ASSIGN_OP
    template <class T, class X, class Y, class Z, class W> 
      vec<VALUE,4> mask4 (const T&in,X,Y,Z,W) {
        if (tsize>X::ref)f[X::ref]=in.getAt(0);
        if (tsize>Y::ref)f[Y::ref]=in.getAt(1);
        if (tsize>Z::ref)f[Z::ref]=in.getAt(2);
        if (tsize>W::ref)f[W::ref]=in.getAt(3);
        return vec<VALUE,4>(getAt(X::ref),
                    getAt(Y::ref),
                    getAt(Z::ref),
                    getAt(W::ref));
    }
    template <class T, class X, class Y, class Z> 
      vec<VALUE,3> mask3 (const T&in,X,Y,Z) {
        if (tsize>X::ref)f[X::ref]=in.getAt(0);
        if (tsize>Y::ref)f[Y::ref]=in.getAt(1);
        if (tsize>Z::ref)f[Z::ref]=in.getAt(2);
        return vec<VALUE,3>(getAt(X::ref),getAt(Y::ref),getAt(Z::ref));
    }
    template <class T, class X, class Y> 
      vec<VALUE,2> mask2 (const T&in,X,Y) {
        if (tsize>X::ref)f[X::ref]=in.getAt(0);
        if (tsize>Y::ref)f[Y::ref]=in.getAt(1);
        return vec<VALUE,2>(getAt(X::ref),getAt(Y::ref));
    }
    template <class T, class X> 
      vec<VALUE,1> mask1 (const T&in,X) {
        if (tsize>X::ref)f[X::ref]=in.getAt(0);
        return vec<VALUE,1>(getAt(X::ref));
    }    
    vec() {
    }
    template <class A,class B, class C, class D> 
      vec (const A &inx, const B &iny, const C &inz, const D& inw) {
        f[0]=inx;
        if (size>1) f[1]=iny;
        if (size>2) f[2]=inz;
        if (size>3) f[3]=inw;
    }
    template <class A, class B, class C> 
      vec (const A& inx, const B& iny, const C& inz) {
        f[0]=inx;if(size>1)f[1]=iny;if(size>2)f[2]=inz;if(size>3)f[3]=VALUE();
    }
    template <class A, class B>vec (const A& inx, const B& iny) {
        f[0]=inx;
        if (size>1) f[1]=iny;
        if (size>2) f[2]=VALUE();
        if (size>3) f[3]=VALUE();
    }
    template <class T> vec (const T& scalar) {
        (*this)=scalar;
    }
    template <class T> 
      vec<typename T::TYPE,T::size> questioncolon(const T &b, const T &c)const {
        return vec<GCCTYPENAME T::TYPE,T::size>
            (singlequestioncolon(getAt(0),b.getAt(0),c.getAt(0)),
             singlequestioncolon(getAt(1),b.getAt(1),c.getAt(1)),
             singlequestioncolon(getAt(2),b.getAt(2),c.getAt(2)),
             singlequestioncolon(getAt(3),b.getAt(3),c.getAt(3)));
    }
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class T>            \
    vec<GCCTYPENAME TYPESPECIFIER<GCCTYPENAME T::TYPE,VALUE>::type, \
        LUB<tsize,T::size>::size> operator op (const T &b)const { \
      return vec<GCCTYPENAME TYPESPECIFIER<GCCTYPENAME T::TYPE,VALUE>::type,LUB<tsize,T::size>::size> \
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
#undef BROOK_BINARY_OP    
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
};












template <class VALUE> class cont{
public:
    typedef VALUE TYPE;
    typedef class cont<VALUE> SELFTYPE;
    enum SIZ{size=1};
protected:
    VALUE f;
public:
    template <class T> operator T () const{
      return T(f);
    }        
    const VALUE &getAt (unsigned int i) const{return f;}
    const SELFTYPE &operator [] (unsigned int i)const {return *this;}
    SELFTYPE &operator [] (unsigned int i) {return *this;}
    SELFTYPE& cast() {
        return *this;
    }
    const SELFTYPE& cast() const{
        return *this;
    }
    template<class T> T castTo() {
        return T(f);
    }
    template<class x, class y, class z, class w> 
      vec<SELFTYPE,4> swizzle4(x,y,z,w)const {
        return vec<SELFTYPE,4>(f,f,f,f);
    }
    template<class x, class y, class z> vec<SELFTYPE,3> swizzle3(x,y,z)const {
        return vec<SELFTYPE,3>(f,f,f);
    }
    template<class x, class y> vec<SELFTYPE,2> swizzle2(x,y)const {
        return vec<SELFTYPE,2>(f,f);
    }
    template<class x> vec<SELFTYPE, 1> swizzle1(x)const {
        return vec<SELFTYPE,1>(f);
    }
#define ASSIGN_OP(op) template <class T> \
         SELFTYPE& operator op (const T & in) {  \
        f op (VALUE)in;  \
        return *this;  \
    }
    ASSIGN_OP(=);
    ASSIGN_OP(/=);
    ASSIGN_OP(+=);
    ASSIGN_OP(-=);
    ASSIGN_OP(*=);
    ASSIGN_OP(%=);
#undef ASSIGN_OP
    template <class T, class X, class Y, class Z, class W> 
      vec<SELFTYPE,4> mask4 (const T&in,X,Y,Z,W) {
        if (size>X::ref)f=in.getAt(0);
        if (size>Y::ref)f=in.getAt(1);
        if (size>Z::ref)f=in.getAt(2);
        if (size>W::ref)f=in.getAt(3);
        return vec<SELFTYPE,4>(getAt(X::ref),
                            getAt(Y::ref),
                            getAt(Z::ref),
                            getAt(W::ref));
    }
    template <class T, class X, class Y, class Z> 
      vec<SELFTYPE,3> mask3 (const T&in,X,Y,Z) {
        if (size>X::ref)f=in.getAt(0);
        if (size>Y::ref)f=in.getAt(1);
        if (size>Z::ref)f=in.getAt(2);
        return vec<SELFTYPE,3>(getAt(X::ref),getAt(Y::ref),getAt(Z::ref));
    }
    template <class T, class X, class Y> 
      vec<SELFTYPE,2> mask2 (const T&in,X,Y) {
        if (size>X::ref)f=in.getAt(0);
        if (size>Y::ref)f=in.getAt(1);
        return vec<SELFTYPE,2>(getAt(X::ref),getAt(Y::ref));
    }
    template <class T, class X> 
      vec<SELFTYPE,1> mask1 (const T&in,X) {
        if (size>X::ref)f=in.getAt(0);
        return vec<SELFTYPE,1>(getAt(X::ref));
    }    
    cont() {
    }
    template <class A,class B, class C, class D> 
      cont (const A &inx, const B &iny, const C &inz, const D& inw) {
        f=inx;
    }
    template <class A, class B, class C> 
      cont (const A& inx, const B& iny, const C& inz) {
        f=inx;
    }
    template <class A, class B> cont (const A& inx, const B& iny) {
        f=inx;
    }
    template <class T> cont (const T& scalar) {
        (*this)=scalar;
    }
    template <class T> 
      vec<typename T::TYPE,T::size> questioncolon(const T &b, const T &c)const {
        return vec<GCCTYPENAME T::TYPE,T::size>
            (singlequestioncolon(f,b.getAt(0),c.getAt(0)),
             singlequestioncolon(f,b.getAt(1),c.getAt(1)),
             singlequestioncolon(f,b.getAt(2),c.getAt(2)),
             singlequestioncolon(f,b.getAt(3),c.getAt(3)));
    }
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class T>            \
    vec<typename T::TYPE,T::size> operator op (const T &b)const { \
      return vec<GCCTYPENAME T::TYPE,T::size> \
                (f op b.getAt(0), \
                 f op b.getAt(1), \
                 f op b.getAt(2), \
                 f op b.getAt(3)); \
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
#undef BROOK_BINARY_OP  
      /*  
#define BROOK_UNARY_OP(op) SELFTYPE operator op ()const { \
      return SELFTYPE(op f);
    }
    BROOK_UNARY_OP(+)
    BROOK_UNARY_OP(-)
    BROOK_UNARY_OP(!)    
#undef BROOK_UNARY_OP
      */
};


template <class T> 
  std::ostream& operator^ (std::ostream& os, const T & a){
    if (T::size==1) {
      GCCTYPENAME T::TYPE x (a.getAt(0));
        os << x;
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

typedef cont<float> __BrtFloat0;
typedef cont<int> __BrtInt0;
typedef cont<char> __BrtChar0;
inline std::ostream&operator <<(std::ostream &a, const cont <float> &b) {
  return a<<b.getAt(0);
}
inline std::ostream&operator <<(std::ostream &a, const cont <char> &b) {
  return a<<b.getAt(0);
}
inline std::ostream&operator <<(std::ostream &a, const cont <int> &b) {
  return a<<b.getAt(0);
}
VECX_CLASS(__BrtFloat1,__BrtFloat0,1);
VECX_CLASS(__BrtFloat2,__BrtFloat0,2);
VECX_CLASS(__BrtFloat3,__BrtFloat0,3);
VECX_CLASS(__BrtFloat4,__BrtFloat0,4);
//VECX_CLASS(__BrtChar0,char,1);
/*
#undef VECX_CLASS
#define VECX_CLASS(NAME,TYPE,X) \
typedef vec<TYPE,X> NAME
*/
VECX_CLASS(__BrtChar1,__BrtChar0,1);
VECX_CLASS(__BrtChar2,__BrtChar0,2);
VECX_CLASS(__BrtChar3,__BrtChar0,3);
VECX_CLASS(__BrtChar4,__BrtChar0,4);
//VECX_CLASS(__BrtInt0,int,1);
VECX_CLASS(__BrtInt1,__BrtInt0,1);
VECX_CLASS(__BrtInt2,__BrtInt0,2);
VECX_CLASS(__BrtInt3,__BrtInt0,3);
VECX_CLASS(__BrtInt4,__BrtInt0,4);
#undef VECX_CLASS
