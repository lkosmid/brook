#ifndef __TYPES_H__
#define __TYPES_H__

#include <math.h>

class Tuple3f;
class Tuple3i;

typedef unsigned char       u8;
typedef signed char         s8;

typedef unsigned short      u16;
typedef signed short        s16;

typedef unsigned int        u32;
typedef signed int          s32;

#ifdef WIN32
#define M_PI  3.14159265358979323846f

typedef unsigned __int64 u64;
typedef signed __int64   s64;
#else
typedef unsigned long long u64;
typedef signed long long s64;
#endif

typedef Tuple3f Point3;
typedef Tuple3f Vector3;
typedef Tuple3f Normal3;
typedef Tuple3f Spectra;

class Tuple3f {
public:
  Tuple3f() : x(0), y(0), z(0) {}
  Tuple3f( float _x, float _y, float _z ) : x(_x), y(_y), z(_z) {}

  //copy constructor.  Very very important...
  Tuple3f(const Tuple3f& t) { x=t.x; y=t.y; z=t.z; }

  ~Tuple3f() {}

  Tuple3f &operator=( const Tuple3f &t )        { x = t.x; y = t.y; z = t.z; return *this; }

  Tuple3f &operator+=( const Tuple3f &t )       { x += t.x; y += t.y; z += t.z; return *this; }
  Tuple3f &operator-=( const Tuple3f &t )       { x -= t.x; y -= t.y; z -= t.z; return *this; }
  Tuple3f &operator*=( const Tuple3f &t )       { x *= t.x; y *= t.y; z *= t.z; return *this; }
  Tuple3f &operator/=( const Tuple3f &t )       { x /= t.x; y /= t.y; z /= t.z; return *this; }
  Tuple3f &operator*=( float f )                { x *= f; y *= f; z *= f; return *this; }
  Tuple3f &operator/=( float f )                { x /= f; y /= f; z /= f; return *this; }

  Tuple3f operator+( const Tuple3f &t ) const   { return Tuple3f( x+t.x, y+t.y, z+t.z ); }
  Tuple3f operator-() const                     { return Tuple3f( -x, -y, -z ); }
  Tuple3f operator-( const Tuple3f &t ) const   { return Tuple3f( x-t.x, y-t.y, z-t.z ); }
  Tuple3f operator*( const Tuple3f &t ) const   { return Tuple3f( x*t.x, y*t.y, z*t.z ); }
  Tuple3f operator/( const Tuple3f &t ) const   { return Tuple3f( x/t.x, y/t.y, z/t.z ); }
  Tuple3f operator+( float f ) const            { return Tuple3f( x+f, y+f, z+f ); }
  Tuple3f operator*( float f ) const            { return Tuple3f( x*f, y*f, z*f ); }
  Tuple3f operator/( float f ) const            { return Tuple3f( x/f, y/f, z/f ); }


  float &operator[]( int i )                    { if (i==0) return x;
                                                  else if (i==1) return y;
                                                  else return z; }

  float Max()                                   { return x > y ? (x > z ? x : z) : (y > z ? y : z); }
  float Min()                                   { return x < y ? (x < z ? x : z) : (y < z ? y : z); }
  int MaxAdr()                                  { return x > y ? (x > z ? 0 : 2) : (y > z ? 1 : 2); }
  int MinAdr()                                  { return x < y ? (x < z ? 0 : 2) : (y < z ? 1 : 2); }

  static Tuple3f Max( const Tuple3f &t1, const Tuple3f &t2 ){
    return Tuple3f( t1.x > t2.x ? t1.x : t2.x,
		    t1.y > t2.y ? t1.y : t2.y,
		    t1.z > t2.z ? t1.z : t2.z );
  }

  static Tuple3f Min( const Tuple3f &t1, const Tuple3f &t2 ){
    return Tuple3f( t1.x < t2.x ? t1.x : t2.x,
		    t1.y < t2.y ? t1.y : t2.y,
		    t1.z < t2.z ? t1.z : t2.z );
  }

  float Length() const                          { return (float) sqrt( x*x + y*y + z*z ); }
  float Length2() const                         { return x*x + y*y + z*z; }
  Tuple3f Normalize() const                     { return (*this)/Length(); }

  float X() const                               { return x; }
  float Y() const                               { return y; }
  float Z() const                               { return z; }
  float R() const                               { return x; }
  float G() const                               { return y; }
  float B() const                               { return z; }

  static float Dot( const Tuple3f &t1, const Tuple3f &t2 ) {
    return t1.x*t2.x + t1.y*t2.y + t1.z*t2.z;
  }

  static Tuple3f Cross( const Tuple3f &t1, const Tuple3f &t2 ) {
    return Tuple3f( t1.y*t2.z - t1.z*t2.y,
                    t1.z*t2.x - t1.x*t2.z,
                    t1.x*t2.y - t1.y*t2.x );
  }

  float x, y, z;

};

class Tuple3i {
public:
  Tuple3i() : x(0), y(0), z(0) {}
  Tuple3i( int _x, int _y, int _z ) : x(_x), y(_y), z(_z) {}
  Tuple3i( const Tuple3f &t ) { x = (int)t.x; y=(int)t.y; z=(int)t.z;}
  Tuple3i( const Tuple3i &t ) { x=t.x; y=t.y; z=t.z; }

  ~Tuple3i() {}

  Tuple3i operator+( const Tuple3i &t ) const   { return Tuple3i( x+t.x, y+t.y, z+t.z ); }
  int &operator[]( int i )                      { if (i==0) return x;
                                                  else if (i==1) return y;
                                                  else return z; }


  int X() const                               { return x; }
  int Y() const                               { return y; }
  int Z() const                               { return z; }
  int R() const                               { return x; }
  int G() const                               { return y; }
  int B() const                               { return z; }

  int x, y, z;
};

#endif /* __TYPES_H__ */
