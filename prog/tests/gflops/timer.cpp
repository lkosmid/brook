
#include <assert.h>

#ifdef _WIN32

#include <windows.h>

static LARGE_INTEGER t;
static float         f;
static int           freq_init = 0;

void resetTimer(void) {
  if (!freq_init) {
    LARGE_INTEGER freq;
    assert (QueryPerformanceFrequency(&freq));
    f = (float) freq.QuadPart;
    freq_init = 1;
  }
  assert (QueryPerformanceCounter(&t));
}

float getTimer(void) {
  LARGE_INTEGER s;
  float d;
  assert (QueryPerformanceCounter(&s));

  d = ((float)(s.QuadPart - t.QuadPart)) / f;

  return (d*1000.0f);
}

#else

#include <sys/time.h>

static struct timeval t;

void resetTimer(void) {
  gettimeofday(&t);
}

float getTimer(void) {
  static struct timeval s;
  gettimeofday(&s);

  return (int) (s.tv_sec - t.tv_sec)*1000.0f + 
    (s.tv_usec - t.tv_sec)/1000.0f;
}


#endif
