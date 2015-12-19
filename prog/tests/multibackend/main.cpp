/* multibackend.cpp
Tests that multiple backends can be used at once
(C) 2007 Niall Douglas
Created: 14th November 2007
*/

#include <stdio.h>
#include <string.h>
#include "brook/brook.hpp"

using namespace brook;

static void threadcode(void *_runtime);
#ifdef WIN32
#include <windows.h>
static DWORD WINAPI _threadcode(LPVOID a)
{
	threadcode(a);
	return 0;
}
#define THREADVAR HANDLE
#define THREADINIT(v, id) (*v=CreateThread(NULL, 0, _threadcode, (LPVOID) id, 0, NULL))
#define THREADSLEEP(v) SleepEx(v, FALSE)
#define THREADWAIT(v) (WaitForSingleObject(v, INFINITE), 0)
#else
#include <pthread.h>
static void *_threadcode(void *a)
{
	threadcode(a);
	return 0;
}
#define THREADVAR pthread_t
#define THREADINIT(v, id) pthread_create(v, NULL, _threadcode, (void *) id)
#define THREADSLEEP(v) usleep(v*1000)
#define THREADWAIT(v) pthread_join(v, NULL)
#endif

extern void  foo (::brook::stream a,
		::brook::stream b);

static struct ThreadRuntime
{
    const char *runtime;
    THREADVAR h;
    int result;
} runtimes[2];
BRTALIGNED float4  input_a[1000][1000];


static void threadcode(void *_runtime)
{
    float4 *output_a=(float4 *) brmalloc(1000*1000*sizeof(float4));
    ThreadRuntime *tr=(ThreadRuntime *) _runtime;
    initialize(tr->runtime);
    {
        stream input_s = stream::create<float4>(1000, 1000);
        stream output_s = stream::create<float4>(1000, 1000);
        streamRead(input_s, input_a);
        foo(output_s, input_s);
        streamWrite(output_s, output_a);
    }
    finalize();
    tr->result=memcmp(input_a, output_a, sizeof(output_a));
    brfree(output_a);
}

int main(void)
{
    int ret=0;
	const char **devs=brook::runtimeTargets(), **dev1, **dev2, **end;
    bool haveCTM=false, haveOGL=false, haveDX9=false;
    for(end=devs; *end; ++end)
    {
        if(!haveCTM && !strncmp(*end, "ctm", 3)) haveCTM=true;
        if(!haveOGL && !strncmp(*end, "ogl", 3)) haveOGL=true;
        if(!haveDX9 && !strncmp(*end, "dx9", 3)) { haveDX9=true; ++end; break; }
    }
    struct BackendResult
    {
        bool isGPU[2], result;
    } backendresults[32];
    memset(backendresults, 0, sizeof(backendresults));
    int n=0, i=0;
    for(dev1=devs; dev1!=end; ++dev1)
    {
        for(dev2=dev1; dev2!=end; ++dev2)
        {
            runtimes[0].runtime=*dev1;
            runtimes[1].runtime=*dev2;
            //printf("Testing %s vs %s\n", *dev1, *dev2);
            THREADINIT(&runtimes[0].h, &runtimes[0]);
            THREADINIT(&runtimes[1].h, &runtimes[1]);
            THREADWAIT(runtimes[0].h);
            THREADWAIT(runtimes[1].h);
            backendresults[n].isGPU[0]=(dev1==devs);
            backendresults[n].isGPU[1]=(dev2==devs);
            backendresults[n].result=(!runtimes[0].result && !runtimes[1].result);
            n++;
        }
    }
    printf("CPU and CPU: %s\n", backendresults[i++].result ? "Yes" : "No");
    bool result=true;
    for(i=1; i<n && !backendresults[i].isGPU[0]; i++)
        result=result && backendresults[i].result;
    printf("CPU and GPU: %s\n", result ? "Yes" : "No");
    result=true;
    for(; i<n; i++)
        result=result && backendresults[i].result;
    printf("GPU and GPU: %s\n", result ? "Yes" : "No");
	return ret;
}
