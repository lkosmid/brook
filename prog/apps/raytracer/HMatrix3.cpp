#include "HMatrix3.h"


HMatrix3::HMatrix3(){
  setIdentity();
}

HMatrix3::~HMatrix3(){}

void HMatrix3::setIdentity(){
  int i,j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      if (i == j)
	data[i][j] = 1.0;
      else
	data[i][j] = 0.0;
}

Tuple3f HMatrix3::apply(const Tuple3f &p){
  float x = data[0][0]*p.x + data[0][1]*p.y + data[0][2]*p.z + data[0][3];
  float y = data[1][0]*p.x + data[1][1]*p.y + data[1][2]*p.z + data[1][3];
  float z = data[2][0]*p.x + data[2][1]*p.y + data[2][2]*p.z + data[2][3];
  float h = data[3][0]*p.x + data[3][1]*p.y + data[3][2]*p.z + data[3][3];
  return Point3(x/h, y/h, z/h);
}

  
HMatrix3 HMatrix3::multiply(const HMatrix3 &m1, const HMatrix3 &m2){
  HMatrix3 m;
  int r,c,i;
  for (r = 0; r < 4; r++)
    for (c = 0; c < 4; c++){
      m.data[r][c] = 0;
      for (i = 0; i < 4; i++)
	m.data[r][c] += m1.data[r][i] * m2.data[i][c];
    }
  return m;
}

void HMatrix3::translate(float tx, float ty, float tz, int dir){
  HMatrix3 m;
  m.data[0][3] = tx;
  m.data[1][3] = ty;
  m.data[2][3] = tz;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}

void HMatrix3::scale(float sx, float sy, float sz, int dir){
  HMatrix3 m;
  m.data[0][0] = sx;
  m.data[1][1] = sy;
  m.data[2][2] = sz;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}

void HMatrix3::rotatex(float theta, int dir){
  HMatrix3 m;
  theta = theta*M_PI/180.0f;
  float ct = cosf(theta);
  float st = sinf(theta);
  m.data[1][1] = ct;
  m.data[1][2] = -st;
  m.data[2][1] = st;
  m.data[2][2] = ct;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}

void HMatrix3::rotatey(float theta, int dir){
  HMatrix3 m;
  theta = theta*M_PI/180.0f;
  float ct = cosf(theta);
  float st = sinf(theta);
  m.data[0][0] = ct;
  m.data[0][2] = st;
  m.data[2][0] = -st;
  m.data[2][2] = ct;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}

void HMatrix3::rotatez(float theta, int dir){
  HMatrix3 m;
  theta = theta*M_PI/180.0f;
  float ct = cosf(theta);
  float st = sinf(theta);
  m.data[0][0] = ct;
  m.data[0][1] = -st;
  m.data[1][0] = st;
  m.data[1][1] = ct;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}

void HMatrix3::rotate(const Vector3 &ax, float theta, int dir) {
  HMatrix3 m;
  Vector3 axis = ax.Normalize();
  float s = sinf(theta);
  float c = cosf(theta);
  
  m.data[0][0] = axis.x * axis.x + (1.f - axis.x * axis.x) * c;
  m.data[0][1] = axis.x * axis.y * (1 - c) - axis.z * s;
  m.data[0][2] = axis.x * axis.z * (1 - c) + axis.y * s;
  m.data[1][0] = axis.x * axis.y * (1 - c) + axis.z * s;
  m.data[1][1] = axis.y * axis.y + (1 - axis.y * axis.y) * c;
  m.data[1][2] = axis.y * axis.z * (1 - c) - axis.x * s;
  m.data[2][0] = axis.x * axis.z * (1 - c) - axis.y * s;
  m.data[2][1] = axis.y * axis.z * (1 - c) + axis.x * s;
  m.data[2][2] = axis.z * axis.z + (1 - axis.z * axis.z) * c;
  if(dir)
    *this = HMatrix3::multiply(*this,m);
  else
    *this = HMatrix3::multiply(m,*this);
}


HMatrix3 HMatrix3::Transpose(HMatrix3 M){
  HMatrix3 temp;
  int i,j;
  for(i=0; i<4; i++)
    for(j=0; j<4; j++)
      temp.data[i][j] = M.data[j][i];
  return temp;
}
