#include <brook.hpp>
#include <stdio.h>
typedef struct Tri_t {
  float3 A;
  float3 B;
  float3 C;
} Tri;

/* Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 *
 * int tri_tri_intersect (float V0[3],float V1[3],float V2[3],
 *                         float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 */

/*
   if USE_EPSILON_TEST is true
     then we do a check:
       if |dv| < EPSILON
         then dv = 0.0;
         else no check is done (which is less robust)
*/
#define USE_EPSILON_TEST	1

const   float EPSILON=.000001f;
float ABS (float x) {
  return x>=0?x:-x;
}

class csVector3
{
public:
  /// The X component of the vector
  float x;
  /// The Y component of the vector
  float y;
  /// The Z component of the vector
  float z;

  /**
   * Make a new vector. The vector is not
   * initialized. This makes the code slightly faster as
   * csVector3 objects are used a lot.
   */
  csVector3 () {}

  /**
   * Make a new initialized vector.
   * Creates a new vector and initializes it to m*<1,1,1>.  To create
   * a vector initialized to the zero vector, use csVector3(0)
   */
  csVector3 (float3 n): x(n.x), y(n.y), z(n.z){}
  csVector3 (float m) : x(m), y(m), z(m) {}

  /// Make a new vector and initialize with the given values.
  csVector3 (float ix, float iy, float iz = 0) : x(ix), y(iy), z(iz) {}

  /// Copy Constructor.
  csVector3 (const csVector3& v) : x(v.x), y(v.y), z(v.z) {}
  /// Conversion from double precision vector to single.
  /// Add two vectors.
  inline csVector3 operator+ (const csVector3& v2) const
  { return csVector3(x+v2.x, y+v2.y, z+v2.z); }

  /// Subtract two vectors.
  inline csVector3 operator- (const csVector3& v2) const
  { return csVector3(x-v2.x, y-v2.y, z-v2.z); }

  /// Subtract two vectors of differing type, cast to double.

  /// Subtract two vectors of differing type, cast to double.

  /// Take the dot product of two vectors.
  inline friend float operator* (const csVector3& v1, const csVector3& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }

  /// Take the cross product of two vectors.
  inline friend csVector3 operator% (const csVector3& v1, const csVector3& v2)
  {
    return csVector3 (v1.y*v2.z-v1.z*v2.y,
                      v1.z*v2.x-v1.x*v2.z,
                      v1.x*v2.y-v1.y*v2.x);
  }

  /// Take cross product of two vectors and put result in this vector.
  void Cross (const csVector3 & px, const csVector3 & py)
  {
    x = px.y*py.z - px.z*py.y;
    y = px.z*py.x - px.x*py.z;
    z = px.x*py.y - px.y*py.x;
  }

  /// Multiply a vector and a scalar.
  inline friend csVector3 operator* (const csVector3& v, float f)
  { return csVector3(v.x*f, v.y*f, v.z*f); }

  /// Multiply a vector and a scalar.
  inline friend csVector3 operator* (float f, const csVector3& v)
  { return csVector3(v.x*f, v.y*f, v.z*f); }


  /// Multiply a vector and a scalar int.
  inline friend csVector3 operator* (const csVector3& v, int f)
  { return v * (float)f; }

  /// Multiply a vector and a scalar int.
  inline friend csVector3 operator* (int f, const csVector3& v)
  { return v * (float)f; }

  /// Divide a vector by a scalar.
  inline friend csVector3 operator/ (const csVector3& v, float f)
  { f = 1.0f/f; return csVector3(v.x*f, v.y*f, v.z*f); }


  /// Divide a vector by a scalar int.
  inline friend csVector3 operator/ (const csVector3& v, int f)
  { return v / (float)f; }

  /// Check if two vectors are equal.
  inline friend bool operator== (const csVector3& v1, const csVector3& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z; }

  /// Check if two vectors are not equal.
  inline friend bool operator!= (const csVector3& v1, const csVector3& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z; }

  /// Project one vector onto another.
  inline friend csVector3 operator>> (const csVector3& v1, const csVector3& v2)
  { return v2*(v1*v2)/(v2*v2); }

  /// Project one vector onto another.
  inline friend csVector3 operator<< (const csVector3& v1, const csVector3& v2)
  { return v1*(v1*v2)/(v1*v1); }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector3& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator> (float f, const csVector3& v)
  { return ABS(v.x)<f && ABS(v.y)<f && ABS(v.z)<f; }

  /// Returns n-th component of the vector.
  inline float operator[] (int n) const { return !n?x:n&1?y:z; }

  /// Returns n-th component of the vector.
  inline float & operator[] (int n) { return !n?x:n&1?y:z; }

  /// Add another vector to this vector.
  inline csVector3& operator+= (const csVector3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
  }

  /// Subtract another vector from this vector.
  inline csVector3& operator-= (const csVector3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
  }

  /// Multiply this vector by a scalar.
  inline csVector3& operator*= (float f)
  { x *= f; y *= f; z *= f; return *this; }

  /// Divide this vector by a scalar.
  inline csVector3& operator/= (float f)
  { f = 1.0f / f; x *= f; y *= f; z *= f; return *this; }

  /// Unary + operator.
  inline csVector3 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector3 operator- () const { return csVector3(-x,-y,-z); }

  /// Set the value of this vector.
  inline void Set (float sx, float sy, float sz) { x = sx; y = sy; z = sz; }

  /// Set the value of this vector.
  inline void Set (const csVector3& v) { x = v.x; y = v.y; z = v.z; }

  /// Returns the norm of this vector.
  float Norm () const;

  /// Return the squared norm (magnitude) of this vector.
  float SquaredNorm () const
  { return x * x + y * y + z * z; }

  /**
   * Returns the unit vector in the direction of this vector.
   * Attempting to normalize a zero-vector will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  csVector3 Unit () const { return (*this)/(this->Norm()); }

  /// Returns the norm (magnitude) of a vector.
  inline static float Norm (const csVector3& v) { return v.Norm(); }

  /// Normalizes a vector to a unit vector.
  inline static csVector3 Unit (const csVector3& v) { return v.Unit(); }

  /// Scale this vector to length = 1.0;
  void Normalize ();

  /// Query if the vector is zero
  inline bool IsZero () const
  { return (x == 0) && (y == 0) && (z == 0); }
};

/* some macros */
#define CROSS(dest, v1, v2)			\
  dest [0] = v1 [1] * v2 [2] - v1 [2] * v2 [1];	\
  dest [1] = v1 [2] * v2 [0] - v1 [0] * v2 [2];	\
  dest [2] = v1 [0] * v2 [1] - v1 [1] * v2 [0];

#define DOT(v1, v2)				\
  (v1 [0] * v2 [0] + v1 [1] * v2 [1] + v1 [2] * v2 [2])

#define SUB(dest, v1, v2)			\
  dest [0] = v1 [0] - v2 [0];			\
  dest [1] = v1 [1] - v2 [1];			\
  dest [2] = v1 [2] - v2 [2];

/* sort so that a <= b */
#define SORT(a, b)				\
  if (a > b)					\
  {						\
    float c;					\
    c = a;					\
    a = b;					\
    b = c;					\
  }

void  ISECT(float VV0, float VV1, float VV2, float D0, float D1, float D2, float&isect0, float&isect1)
  {						
    isect0 = VV0 + (VV1 - VV0) * D0 / (D0 - D1);
    isect1 = VV0 + (VV2 - VV0) * D0 / (D0 - D2);
  }

void COMPUTE_INTERVALS(float VV0, float VV1, float VV2,float D0,float D1,float D2,float D0D1,float D0D2,float& isect0,float& isect1) {
  if (D0D1 > 0.0f)					
    /* here we know that D0D2<=0.0 */			
    /* that is D0, D1 are on the same side, */		
    /* D2 on the other or on the plane */		
    ISECT (VV2, VV0, VV1, D2, D0, D1, isect0, isect1)	;
  else if (D0D2 > 0.0f)					
    /* here we know that d0d1 <= 0.0 */			
    ISECT (VV1, VV0, VV2, D1, D0, D2, isect0, isect1)	;
  else if (D1 * D2 > 0.0f || D0 != 0.0f)		
    /* here we know that d0d1<=0.0 or that D0!=0.0 */	
    ISECT (VV0, VV1, VV2, D0, D1, D2, isect0, isect1)	;
  else if (D1 != 0.0f)					
    ISECT (VV1, VV0, VV2, D1, D0, D2, isect0, isect1)	;
  else if(D2!=0.0f)                                     
    ISECT (VV2, VV0, VV1, D2, D0, D1, isect0, isect1)	;
  else							
    /* triangles are coplanar */			
    //    if (coplanar_tri_tri (N1, V0, V1, V2, U0, U1, U2))
      assert(0);
}
/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0, U0, U1)			\
  Bx = U0 [i0] - U1 [i0];				\
  By = U0 [i1] - U1 [i1];				\
  Cx = V0 [i0] - U0 [i0];				\
  Cy = V0 [i1] - U0 [i1];				\
  f = Ay * Bx - Ax * By;				\
  d = By * Cx - Bx * Cy;				\
  if ((f > 0 && d >= 0 && d <= f)			\
   || (f < 0 && d <= 0 && d >= f))			\
  {							\
    e = Ax * Cy - Ay * Cx;				\
    if (f > 0)						\
    {							\
      if (e >= 0 && e <= f) return 1;			\
    }							\
    else						\
    {							\
      if (e <= 0 && e >= f) return 1;			\
    }							\
  }

#define EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2)	\
  {							\
    float Ax, Ay, Bx, By, Cx, Cy, e, d, f;		\
    Ax = V1 [i0] - V0 [i0];				\
    Ay = V1 [i1] - V0 [i1];				\
    /* test edge U0,U1 against V0,V1 */			\
    EDGE_EDGE_TEST (V0, U0, U1);			\
    /* test edge U1,U2 against V0,V1 */			\
    EDGE_EDGE_TEST (V0, U1, U2);			\
    /* test edge U2,U1 against V0,V1 */			\
    EDGE_EDGE_TEST (V0, U2, U0);			\
  }

#define POINT_IN_TRI(V0, U0, U1, U2)			\
  {							\
    float a, b, c, d0, d1, d2;				\
    /* is T1 completly inside T2? */			\
    /* check if V0 is inside tri(U0,U1,U2) */		\
    a = U1 [i1] - U0 [i1];				\
    b = -(U1 [i0] - U0 [i0]);				\
    c = -a * U0 [i0] - b * U0 [i1];			\
    d0 = a * V0 [i0] + b * V0 [i1] + c;			\
							\
    a = U2 [i1] - U1 [i1];				\
    b = -(U2 [i0] - U1 [i0]);				\
    c = -a * U1 [i0] - b * U1 [i1];			\
    d1 = a * V0 [i0] + b * V0 [i1] + c;			\
							\
    a = U0 [i1] - U2 [i1];				\
    b = -(U0 [i0] - U2 [i0]);				\
    c = -a * U2 [i0] - b * U2 [i1];			\
    d2 = a * V0 [i0] + b * V0 [i1] + c;			\
    if (d0 * d1 > 0.0)					\
    {							\
      if (d0 * d2 > 0.0) return 1;			\
    }							\
  }

int coplanar_tri_tri (float N[3],
  const csVector3 &V0, const csVector3 &V1, const csVector3 &V2,
  const csVector3 &U0, const csVector3 &U1, const csVector3 &U2)
{
  float A[3];
  short i0,i1;
  /* first project onto an axis-aligned plane, that maximizes the area */
  /* of the triangles, compute indices: i0,i1. */
  A [0] = ABS  (N [0]);
  A [1] = ABS  (N [1]);
  A [2] = ABS  (N [2]);
  if (A [0] > A [1])
  {
    if (A [0] > A [2])
    {
      i0 = 1;			/* A[0] is greatest */
      i1 = 2;
    }
    else
    {
      i0 = 0;			/* A[2] is greatest */
      i1 = 1;
    }
  }
  else				/* A[0]<=A[1] */
  {
    if (A [2] > A [1])
    {
      i0 = 0;			/* A[2] is greatest */
      i1 = 1;
    }
    else
    {
      i0 = 0;			/* A[1] is greatest */
      i1 = 2;
    }
  }

  /* test all edges of triangle 1 against the edges of triangle 2 */
  EDGE_AGAINST_TRI_EDGES (V0, V1, U0, U1, U2);
  EDGE_AGAINST_TRI_EDGES (V1, V2, U0, U1, U2);
  EDGE_AGAINST_TRI_EDGES (V2, V0, U0, U1, U2);

  /* finally, test if tri1 is totally contained in tri2 or vice versa */
  POINT_IN_TRI (V0, U0, U1, U2);
  POINT_IN_TRI (U0, V0, V1, V2);

  return 0;
}

int tri_contact(const csVector3 &V0, const csVector3 &V1, const csVector3 &V2,
                const csVector3 &U0, const csVector3 &U1, const csVector3 &U2)
{
  float E1 [3], E2 [3];
  float N1 [3], N2 [3], d1, d2;
  float du0, du1, du2, dv0, dv1, dv2;
  float D [3];
  float isect1 [2], isect2 [2];
  float du0du1, du0du2, dv0dv1, dv0dv2;
  short index;
  float vp0, vp1, vp2;
  float up0, up1, up2;
  float b, c, max;

  /* compute plane equation of triangle(V0,V1,V2) */
  SUB (E1, V1, V0);
  SUB (E2, V2, V0);
  CROSS (N1, E1, E2);
  d1 = -DOT (N1, V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to
     the plane */
  du0 = DOT (N1, U0) + d1;
  du1 = DOT (N1, U1) + d1;
  du2 = DOT (N1, U2) + d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST
  if (ABS  (du0) < EPSILON) du0 = 0.0;
  if (ABS  (du1) < EPSILON) du1 = 0.0;
  if (ABS  (du2) < EPSILON) du2 = 0.0;
#endif
  du0du1 = du0 * du1;
  du0du2 = du0 * du2;

  /* same sign on all of them + not equal 0 ? */
  if (du0du1 > 0.0f && du0du2 > 0.0f)
    return 0;			/* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB (E1, U1, U0);
  SUB (E2, U2, U0);
  CROSS (N2, E1, E2);
  d2 = -DOT (N2, U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0 = DOT (N2, V0) + d2;
  dv1 = DOT (N2, V1) + d2;
  dv2 = DOT (N2, V2) + d2;

#if USE_EPSILON_TEST
  if (ABS  (dv0) < EPSILON) dv0 = 0.0;
  if (ABS  (dv1) < EPSILON) dv1 = 0.0;
  if (ABS  (dv2) < EPSILON) dv2 = 0.0;
#endif

  dv0dv1 = dv0 * dv1;
  dv0dv2 = dv0 * dv2;

  /* same sign on all of them + not equal 0 ? */
  if (dv0dv1 > 0.0f && dv0dv2 > 0.0f)
    return 0;			/* no intersection occurs */

  /* compute direction of intersection line */
  CROSS (D, N2, N1);

  /* compute and index to the largest component of D */
  max = ABS  (D [0]);
  index = 0;
  b = ABS  (D [1]);
  c = ABS  (D [2]);
  if (b > max) max = b, index = 1;
  if (c > max) max = c, index = 2;

  /* this is the simplified projection onto L*/
  vp0 = V0 [index];
  vp1 = V1 [index];
  vp2 = V2 [index];

  up0 = U0 [index];
  up1 = U1 [index];
  up2 = U2 [index];

  /* compute interval for triangle 1 */
  COMPUTE_INTERVALS (
    vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, isect1 [0], isect1 [1]);

  /* compute interval for triangle 2 */
  COMPUTE_INTERVALS (
    up0, up1, up2, du0, du1, du2, du0du1, du0du2, isect2 [0], isect2 [1]);

  SORT (isect1 [0], isect1 [1]);
  SORT (isect2 [0], isect2 [1]);

  if (isect1 [1] < isect2 [0] || isect2 [1] < isect1 [0])
    return 0;
  return 1;
}


float min3(float a, float b, float c) {
  return a>b?(b>c?c:b):a;
}
float max3(float a, float b, float c) {
  return a<b?(b<c?c:b):a;
}

int project6 (csVector3 ax, csVector3 p1, csVector3 p2, csVector3 p3,
  csVector3 q1, csVector3 q2, csVector3 q3)
{
  float P1 = ax * p1;
  float P2 = ax * p2;
  float P3 = ax * p3;
  float Q1 = ax * q1;
  float Q2 = ax * q2;
  float Q3 = ax * q3;

  float mx1 = max3 (P1, P2, P3);
  float mn1 = min3 (P1, P2, P3);
  float mx2 = max3 (Q1, Q2, Q3);
  float mn2 = min3 (Q1, Q2, Q3);

  if (mn1 > mx2) return 0;
  if (mn2 > mx1) return 0;
  return 1;
}

bool tri_contact_nodiv (csVector3 P1, csVector3 P2, csVector3 P3,
                        csVector3 Q1, csVector3 Q2, csVector3 Q3)
{
  //
  // One triangle is (p1,p2,p3).  Other is (q1,q2,q3).
  // Edges are (e1,e2,e3) and (f1,f2,f3).
  // Normals are n1 and m1
  // Outwards are (g1,g2,g3) and (h1,h2,h3).
  //
  // We assume that the triangle vertices are in the same coordinate system.
  //
  // First thing we do is establish a new c.s. so that p1 is at (0,0,0).
  //

  csVector3 p1 = P1 - P2;
  csVector3 p2 = P2 - P1;
  csVector3 p3 = P3 - P1;

  csVector3 q1 = Q1 - P1;
  csVector3 q2 = Q2 - P1;
  csVector3 q3 = Q3 - P1;

  csVector3 e1 = p2 - p1;
  csVector3 e2 = p3 - p2;
  csVector3 e3 = p1 - p3;

  csVector3 f1 = q2 - q1;
  csVector3 f2 = q3 - q2;
  csVector3 f3 = q1 - q3;

  csVector3 n1 = e1 % e2;
  csVector3 m1 = f1 % f2;

  csVector3 g1 = e1 % n1;
  csVector3 g2 = e2 % n1;
  csVector3 g3 = e3 % n1;
  csVector3 h1 = f1 % m1;
  csVector3 h2 = f2 % m1;
  csVector3 h3 = f3 % m1;

  csVector3 ef11 = e1 % f1;
  csVector3 ef12 = e1 % f2;
  csVector3 ef13 = e1 % f3;

  csVector3 ef21 = e2 % f1;
  csVector3 ef22 = e2 % f2;
  csVector3 ef23 = e2 % f3;

  csVector3 ef31 = e3 % f1;
  csVector3 ef32 = e3 % f2;
  csVector3 ef33 = e3 % f3;

  // now begin the series of tests

  if (!project6(n1, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(m1, p1, p2, p3, q1, q2, q3)) return false;

  if (!project6(ef11, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef12, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef13, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef21, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef22, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef23, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef31, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef32, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(ef33, p1, p2, p3, q1, q2, q3)) return false;

  if (!project6(g1, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(g2, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(g3, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(h1, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(h2, p1, p2, p3, q1, q2, q3)) return false;
  if (!project6(h3, p1, p2, p3, q1, q2, q3)) return false;

  return true;
}



extern void doTest(float * matrix,
                   int sizex,int sizey,
                   float3 * hitsa,
                   float3* hitsb,
                   float4 * ohits);

extern void doTestNoDiv(float * matrix,
                        int sizex,int sizey,
                        float3 * hitsa,
                        float3* hitsb,
                        float4 * ohits);
#define SIZEX 1
#define SIZEY 1
void SetTriangles(int size, float3* a, float3 *b) {
  float rm = RAND_MAX*.25;
  for (int i=0;i<size;++i) {
  a[i*3]=float3(rand()/rm,rand()/rm,rand()/rm);
  a[i*3+1]=float3(-1-rand()/rm,-1-rand()/rm,-1-rand()/rm);
  a[i*3+2]=float3(-1-rand()/rm,-1-rand()/rm,-1-rand()/rm);
  b[i*3]=float3(-1-rand()/rm,-1-rand()/rm,-1-rand()/rm);
  b[i*3+1]=float3(-1-rand()/rm,-1-rand()/rm,-1-rand()/rm);
  b[i*3+2]=float3(-1-rand()/rm,-1-rand()/rm,-1-rand()/rm);
  }
}
int main (int argc, char ** argv) {
  float3 * a= (float3*)malloc(sizeof(float3)*3*SIZEX*SIZEY);
  float3 * b= (float3*)malloc(sizeof(float3)*3*SIZEX*SIZEY);
  float4 * rez = (float4*) malloc (sizeof(float4)*SIZEX*SIZEY);
  float4 * rez_nodiv = (float4*) malloc (sizeof(float4)*SIZEX*SIZEY);
  float matrix[16]={1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1};
  srand(1);
  for (unsigned int k=0;k<12;++k) {
  SetTriangles(SIZEX*SIZEY,a,b);
  }
  int counter=0;
  int counter2=0;
  for (unsigned int j=0;j<1024*64;++j) {
  SetTriangles(SIZEX*SIZEY,a,b);
  doTest(matrix,SIZEX,SIZEY,a,b,rez);
  doTestNoDiv(matrix,SIZEX,SIZEY,a,b,rez_nodiv);
  for (unsigned int i=0;i<SIZEX*SIZEY;++i) {
    bool hit = (rez[i].x!=-1);
    bool khitnodiv = (rez_nodiv[i].x!=-1);
    bool hitnodiv = tri_contact_nodiv(a[i*3],a[i*3+1],a[i*3+2],
                                      b[i*3],b[i*3+1],b[i*3+2])?1:0;
    bool hitdiv = tri_contact(a[i*3],a[i*3+1],a[i*3+2],
                              b[i*3],b[i*3+1],b[i*3+2])?1:0;
    assert(hit==hitdiv);
    //    assert(khitnodiv==hitnodiv);
    if (hit!=hitnodiv)
      counter2++;
    if (hit&&!hitnodiv) {
      counter--;
    }
    if (hitnodiv&&!hit) {
      counter++;
    }
  }
  }
  printf ("Num differences btw dif and nodiv %d %d\n",counter2, counter);
  free (a);
  free (b);
  free (rez);
  return 0;
}
