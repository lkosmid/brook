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

template <> class LCM<vec<int,2>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<int,2> > {public:
    typedef int type;
};
template <> class LCM<vec<int,2>,float> {public:
    typedef float type;
};
template <> class LCM<int,vec<float,2> > {public:
    typedef float type;
};
template <> class LCM<float,vec<int,2> > {public:
    typedef float type;
};
template <> class LCM<vec<float,2>,int > {public:
    typedef float type;
};
template <> class LCM<vec<float,2>,float> {public:
    typedef float type;
};
template <> class LCM<float,vec<float,2> > {public:
    typedef float type;
};

template <> class LCM<char,vec<int,2> > {public:
    typedef int type;
};
template <> class LCM<vec<char,2>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<char,2> > {public:
    typedef int type;
};
template <> class LCM<vec<int,2>,char> {public:
    typedef int type;
};
template <> class LCM<char,vec<char,2> > {public:
    typedef char type;
};
template <> class LCM<vec<char,2>,char> {public:
    typedef char type;
};
template <> class LCM<float,vec<char,2> > {public:
    typedef float type;
};
template <> class LCM<vec<float,2>,char> {public:
    typedef float type;
};
template <> class LCM<vec<char,2>,float> {public:
    typedef float type;
};
template <> class LCM<char,vec<float,2> > {public:
    typedef float type;
};
template <> class LCM<vec<int,3>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<int,3> > {public:
    typedef int type;
};
template <> class LCM<vec<int,3>,float> {public:
    typedef float type;
};
template <> class LCM<int,vec<float,3> > {public:
    typedef float type;
};
template <> class LCM<float,vec<int,3> > {public:
    typedef float type;
};
template <> class LCM<vec<float,3>,int > {public:
    typedef float type;
};
template <> class LCM<vec<float,3>,float> {public:
    typedef float type;
};
template <> class LCM<float,vec<float,3> > {public:
    typedef float type;
};

template <> class LCM<char,vec<int,3> > {public:
    typedef int type;
};
template <> class LCM<vec<char,3>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<char,3> > {public:
    typedef int type;
};
template <> class LCM<vec<int,3>,char> {public:
    typedef int type;
};
template <> class LCM<char,vec<char,3> > {public:
    typedef char type;
};
template <> class LCM<vec<char,3>,char> {public:
    typedef char type;
};
template <> class LCM<float,vec<char,3> > {public:
    typedef float type;
};
template <> class LCM<vec<float,3>,char> {public:
    typedef float type;
};
template <> class LCM<vec<char,3>,float> {public:
    typedef float type;
};
template <> class LCM<char,vec<float,3> > {public:
    typedef float type;
};


template <> class LCM<vec<int,4>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<int,4> > {public:
    typedef int type;
};
template <> class LCM<vec<int,4>,float> {public:
    typedef float type;
};
template <> class LCM<int,vec<float,4> > {public:
    typedef float type;
};
template <> class LCM<float,vec<int,4> > {public:
    typedef float type;
};
template <> class LCM<vec<float,4>,int > {public:
    typedef float type;
};
template <> class LCM<vec<float,4>,float> {public:
    typedef float type;
};
template <> class LCM<float,vec<float,4> > {public:
    typedef float type;
};

template <> class LCM<char,vec<int,4> > {public:
    typedef int type;
};
template <> class LCM<vec<char,4>,int> {public:
    typedef int type;
};
template <> class LCM<int,vec<char,4> > {public:
    typedef int type;
};
template <> class LCM<vec<int,4>,char> {public:
    typedef int type;
};
template <> class LCM<char,vec<char,4> > {public:
    typedef char type;
};
template <> class LCM<vec<char,4>,char> {public:
    typedef char type;
};
template <> class LCM<float,vec<char,4> > {public:
    typedef float type;
};
template <> class LCM<vec<float,4>,char> {public:
    typedef float type;
};
template <> class LCM<vec<char,4>,float> {public:
    typedef float type;
};
template <> class LCM<char,vec<float,4> > {public:
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


template <> class COMMON_CHAR<vec<int,2>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<int,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,2>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<float,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<int,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,2>,int > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,2>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<float,2> > {public:
    typedef char type;
};

template <> class COMMON_CHAR<char,vec<int,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,2>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<char,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,2>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<char,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,2>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<char,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,2>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,2>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<float,2> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,3>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<int,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,3>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<float,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<int,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,3>,int > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,3>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<float,3> > {public:
    typedef char type;
};

template <> class COMMON_CHAR<char,vec<int,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,3>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<char,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,3>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<char,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,3>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<char,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,3>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,3>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<float,3> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,4>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<int,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,4>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<float,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<int,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,4>,int > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,4>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<float,4> > {public:
    typedef char type;
};

template <> class COMMON_CHAR<char,vec<int,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,4>,int> {public:
    typedef char type;
};
template <> class COMMON_CHAR<int,vec<char,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<int,4>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<char,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,4>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<float,vec<char,4> > {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<float,4>,char> {public:
    typedef char type;
};
template <> class COMMON_CHAR<vec<char,4>,float> {public:
    typedef char type;
};
template <> class COMMON_CHAR<char,vec<float,4> > {public:
    typedef char type;
};
