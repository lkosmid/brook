#ifndef BROOK_H
#define BROOK_H

class __BrookRunTime;
class __BrookKernel;
class __BrookStream;
class __BrookIntArray;

#define __MAXPROGLENGTH (1024*32)

typedef struct {
  float x,y;
} float2;

typedef struct {
  float x,y,z;
} float3;

typedef struct {
  float x,y,z,w;
} float4;


class __BrookIntArray {
public:
  __BrookIntArray(...);
  int *v;
  ~__BrookIntArray();
};

class __BrookRunTime {
public:
  __BrookRunTime();
  virtual __BrookKernel *LoadKernel(const char[][__MAXPROGLENGTH]);
  virtual __BrookStream *CreateStream(const char type[],
				      const __BrookIntArray & dims);
  virtual ~__BrookRunTime();
};

class __BrookKernel {
public:
  __BrookKernel();
  virtual void SetInput(const int arg, const __BrookStream *s);
  virtual void SetConstantFloat(const int arg, const float &val);  
  virtual void SetConstantFloat2(const int arg, const float2 &val);  
  virtual void SetConstantFloat3(const int arg, const float3 &val); 
  virtual void SetConstantFloat4(const int arg, const float4 &val);
  virtual void SetGatherInput(const int arg, const __BrookStream *s);
  virtual void SetOutput(const __BrookStream *s);
  virtual void Exec(void);
  virtual ~__BrookKernel();
};

class __BrookStream {
public:
  __BrookStream (const char type[], const __BrookIntArray &dims);
  virtual void streamRead(void *p);
  virtual void streamWrite(void *p);
  virtual ~__BrookStream();
};

inline static void streamRead(__BrookStream *s, void *p) {
  s->streamRead(p);
}

inline static void streamWrite(__BrookStream *s, void *p) {
  s->streamWrite(p);
}

extern __BrookRunTime *__brt;

#endif
