#ifndef BROOK_H
#define BROOK_H

class __BrookRunTime;
class __BrookKernel;
class __BrookStream;
class __BrookIntArray;

#define __MAXPROGLENGTH (1024*32)
#define __MAXSTREAMDIMS (8)

typedef struct {
  float x,y;
} float2;

typedef struct {
  float x,y,z;
} float3;

typedef struct {
  float x,y,z,w;
} float4;

class __BrookRunTime {
public:
  __BrookRunTime() {}
  virtual __BrookKernel *LoadKernel(const char*[]) = 0;
  virtual __BrookStream *CreateStream(const char type[], int dims, int extents[]) = 0;
  virtual ~__BrookRunTime() {}
};

class __BrookKernel {
public:
  __BrookKernel() {}
  virtual void SetInput(const int arg, const __BrookStream *s) = 0;
  virtual void SetConstantFloat(const int arg, const float &val) = 0;  
  virtual void SetConstantFloat2(const int arg, const float2 &val) = 0;  
  virtual void SetConstantFloat3(const int arg, const float3 &val) = 0; 
  virtual void SetConstantFloat4(const int arg, const float4 &val) = 0;
  virtual void SetGatherInput(const int arg, const __BrookStream *s) = 0;
  virtual void SetOutput(const __BrookStream *s) = 0;
  virtual void Exec(void) = 0;
  virtual ~__BrookKernel() {}
};

class __BrookStream {
public:
  __BrookStream (const char type[], int dims, int extents[]) {}
  virtual void streamRead(void *p) = 0;
  virtual void streamWrite(void *p) = 0;
  virtual ~__BrookStream() {}
};

inline static void streamRead(__BrookStream *s, void *p) {
  s->streamRead(p);
}

inline static void streamWrite(__BrookStream *s, void *p) {
  s->streamWrite(p);
}

__BrookStream* CreateStream(const char type[], ... );

__BrookRunTime* __brt();

#endif
