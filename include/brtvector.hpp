#include <iostream>
#include <math.h>
 
template <class T, class B> static T singlequestioncolon (const B& a, const T&b, const T&c){
	return a.questioncolon(b,c);
};
template <> static float singlequestioncolon (const char & a, const float &b, const float&c) {
	return a?b:c;
}
template <> static float singlequestioncolon (const float & a, const float &b, const float&c) {
	return a?b:c;
}

template <unsigned int sizeA, unsigned int sizeB> class LUB {
public:
	const static unsigned int size  = sizeA==1?sizeB:sizeB==1?sizeA:sizeA>sizeB?sizeB:sizeA;
};
template <class VALUE=float, unsigned int tsize=4> class vec;

template <class A, class B> class LCM {
public:
	typedef vec<typename LCM<typename A::TYPE , typename B::TYPE>::type,LUB<A::size,B::size>::size > type;
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


template <class A, class B> class COMMON_CHAR {
public:
	typedef vec<typename COMMON_CHAR<typename A::TYPE , typename B::TYPE>::type,LUB<A::size,B::size>::size > type;
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
template <class T> static typename GetValueOf<T>::type GetAt (const T & in,int i) {
	return in.getAt(i);
}
template <> static float GetAt (const float & in, int i) {
	return in;
}
template <> static double GetAt (const double & in, int i) {
	return in;
}
template <> static char GetAt(const char & in, int i) {
	return in;
}
template <> static int GetAt(const int & in, int i) {
	return in;
}
class MaskX {public:
	const static unsigned int ref=0;
};
class MaskY {public:
	const static unsigned int ref=1;
};
class MaskZ {public:
	const static unsigned int ref=2;
};
class MaskW {public:
	const static unsigned int ref=3;
};
static MaskX maskX;
static MaskY maskY;
static MaskZ maskZ;
static MaskW maskW;

template <class T> class InitializeClass {public:
	template <class V> T operator () (const V&a, const V&b, const V&c, const V&d) {
		return T(a,b,c,d);
	}
};
template <> class InitializeClass<bool> {public:
	template <class V> float operator () (const V&a, const V&b, const V&c, const V&d) {return a;}
};
template <> class InitializeClass<float> {public:
	template <class V> float operator () (const V&a, const V&b, const V&c, const V&d) {return a;}
};
template <> class InitializeClass<double> {public:
	template <class V> double operator () (const V&a, const V&b, const V&c, const V&d) {return a;}
};
template <> class InitializeClass<int> {public:
	template <class V> int operator () (const V&a, const V&b, const V&c, const V&d) {return (int)a;}
};
template <> class InitializeClass<unsigned int> {public:
	template <class V> int operator () (const V&a, const V&b, const V&c, const V&d) {return (int)a;}
};
template <> class InitializeClass<char> {public:
	template <class V> char operator () (const V&a, const V&b, const V&c, const V&d) {return (char)a;}
};


template <class VALUE, unsigned int tsize> class vec{
public:
	typedef VALUE TYPE;
	const static unsigned int size=tsize;
protected:
	VALUE f[size];
public:
	template <class T> operator T () const{InitializeClass<T> a; return a(getAt(0),getAt(1),getAt(2),getAt(3));}	
	
	const VALUE &getAt (unsigned int i) const{return f[i%size];}
	const VALUE &operator [] (unsigned int i)const {return f[i%size];}
	VALUE &operator [] (unsigned int i) {return f[i%size];}	
	template<class T> T cast() {
		InitializeClass<T> a; return a(getAt(0),getAt(1),getAt(2),getAt(3));
	}
	template<int x, int y, int z, int w> vec<VALUE,4> swizzle4()const {
		return vec<VALUE,4>(getAt(x),getAt(y),getAt(z),getAt(w));
	}
	template<int x, int y, int z> vec<VALUE,3> swizzle3()const {
		return vec<VALUE,3>(getAt(x),getAt(y),getAt(z),0);
	}
	template<int x, int y> vec<VALUE,2> swizzle2()const {
		return vec<VALUE,2>(getAt(x),getAt(y),0,0);
	}
	template<int x> vec<VALUE, 1> swizzle1()const {
		return vec<VALUE,1>(getAt(x),0,0,0);
	}
#define ASSIGN_OP(op) template <class T> vec<VALUE,tsize>& operator op (const T & in) {  \
		f[0] op GetAt<T>(in,0);  \
		if (size>1) f[1] op GetAt<T>(in,1);  \
		if (size>2) f[2] op GetAt<T>(in,2);  \
		if (size>3) f[3] op GetAt<T>(in,3);  \
		return *this;  \
	}
	ASSIGN_OP(=);
	ASSIGN_OP(/=);
	ASSIGN_OP(+=);
	ASSIGN_OP(-=);
	ASSIGN_OP(*=);
	ASSIGN_OP(%=);
#undef ASSIGN_OP
	template <class T, class X, class Y, class Z, class W> vec<VALUE,4> mask4 (const T&in,X,Y,Z,W) {
		if (size>X::ref)f[X::ref]=in.getAt(0);
		if (size>Y::ref)f[Y::ref]=in.getAt(1);
		if (size>Z::ref)f[Z::ref]=in.getAt(2);
		if (size>W::ref)f[W::ref]=in.getAt(3);
		return vec<VALUE,4>(getAt(X::ref),getAt(Y::ref),getAt(Z::ref),getAt(W::ref));
	}
	template <class T, class X, class Y, class Z> vec<VALUE,3> mask3 (const T&in,X,Y,Z) {
		if (size>X::ref)f[X::ref]=in.getAt(0);
		if (size>Y::ref)f[Y::ref]=in.getAt(1);
		if (size>Z::ref)f[Z::ref]=in.getAt(2);
		return vec<VALUE,3>(getAt(X::ref),getAt(Y::ref),getAt(Z::ref));
	}
	template <class T, class X, class Y> vec<VALUE,2> mask2 (const T&in,X,Y) {
		if (size>X::ref)f[X::ref]=in.getAt(0);
		if (size>Y::ref)f[Y::ref]=in.getAt(1);
		return vec<VALUE,2>(getAt(X::ref),getAt(Y::ref));
	}
	template <class T, class X> vec<VALUE,1> mask1 (const T&in,X) {
		if (size>X::ref)f[X::ref]=in.getAt(0);
		return vec<VALUE,1>(getAt(X::ref));
	}
	
	vec() {for (unsigned int i=0;i<size;++i) f[i]=VALUE();}
	template <class A,class B, class C, class D> vec (const A &inx, const B &iny, const C &inz, const D& inw) {
		f[0]=inx;if (size>1) f[1]=iny;if(size>2)f[2]=inz;if (size>3)f[3]=inw;
	}
	template <class A, class B, class C> vec (const A& inx, const B& iny, const C& inz) {
		f[0]=inx;if (size>1) f[1]=iny;if(size>2)f[2]=inz;if (size>3)f[3]=VALUE();
	}
	template <class A, class B>vec (const A& inx, const B& iny) {
		f[0]=inx;if (size>1) f[1]=iny;if(size>2)f[2]=VALUE();if (size>3)f[3]=VALUE();
	}
	template <class T> vec (const T& scalar) {
		(*this)=scalar;//if (size>1) f[1]=scalar;if(size>2)f[2]=scalar;if (size>3)f[3]=scalar;
	}
	template <class T> vec<typename T::TYPE,tsize> questioncolon(const T & b, const T & c)const {
		return vec<typename T::TYPE,tsize>
			(singlequestioncolon(getAt(0),b.getAt(0),c.getAt(0)),
			 singlequestioncolon(getAt(1),b.getAt(1),c.getAt(1)),
			 singlequestioncolon(getAt(2),b.getAt(2),c.getAt(2)),
			 singlequestioncolon(getAt(3),b.getAt(3),c.getAt(3)));
	}
#define BROOK_BINARY_OP(op,TYPESPECIFIER) template <class T> \
vec<typename TYPESPECIFIER<typename T::TYPE,VALUE>::type,LUB<T::size,tsize>::size> operator op (const T &b)const { \
	return vec<typename TYPESPECIFIER<typename T::TYPE,VALUE>::type,LUB<T::size,tsize>::size > \
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
#define BROOK_UNARY_OP(op) vec<VALUE,tsize> operator op ()const { return vec<VALUE, tsize > (op getAt(0), op getAt(1) ,op getAt(2), op getAt(3));}
	BROOK_UNARY_OP(+)
	BROOK_UNARY_OP(-)
	BROOK_UNARY_OP(!)	
#undef BROOK_UNARY_OP
};



template <class T> std::ostream& operator^ (std::ostream& os, const T & a){
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

#define VECX_CLASS(NAME,TYPE,X) inline std::ostream& operator << (std::ostream&a,const vec<TYPE,X> & b){return a^b;}   \
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

#define MATRIXXY_CLASS(TYPE,X,Y) inline std::ostream& operator << (std::ostream&a,const vec<vec<TYPE##0,X>,Y> & b){return a^b;}   \
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
