#include <map>
#include <string>
#include <iostream>
#include "cpu.hpp"
using std::map;
using std::string;
static map<string,int> getKnownTypes() {
    map<string,int>ret;
    ret.insert(map<string,int>::value_type("float",sizeof(float)));
    ret.insert(map<string,int>::value_type("float1",sizeof(float)));
    ret.insert(map<string,int>::value_type("float2",sizeof(float2)));
    ret.insert(map<string,int>::value_type("float3",sizeof(float3)));
    ret.insert(map<string,int>::value_type("float4",sizeof(float4)));            
    ret.insert(map<string,int>::value_type("int",sizeof(int)));
    ret.insert(map<string,int>::value_type("int1",sizeof(int)));
    ret.insert(map<string,int>::value_type("int2",2*sizeof(int)));
    ret.insert(map<string,int>::value_type("int3",3*sizeof(int)));
    ret.insert(map<string,int>::value_type("int4",4*sizeof(int)));            
    ret.insert(map<string,int>::value_type("bool",sizeof(bool)));
    ret.insert(map<string,int>::value_type("bool1",sizeof(bool)));
    ret.insert(map<string,int>::value_type("bool2",2*sizeof(bool)));
    ret.insert(map<string,int>::value_type("bool3",3*sizeof(bool)));
    ret.insert(map<string,int>::value_type("bool4",4*sizeof(bool)));            
    return ret;
}
static map<string,int> knownTypes = getKnownTypes();
namespace brook{
    CPUStream::CPUStream(CPURunTime *, const char type[], int dims, int extents[]){
	this->extents = (unsigned int *)malloc(dims*sizeof(unsigned int));
	totalsize=1;
	for(int i=0;i<dims;++i) {
	    this->extents[i]=extents[i];
	    totalsize*=extents[i];
	}
	stride=knownTypes[type];
	if (stride) {
	    data = malloc(stride*totalsize);
	}else {
	    std::cerr<<"Failure to produce stream of type "<<type<<std::endl;
	}
    }
    void CPUStream::Read(const void*inData) {
	memcpy(data,inData,stride*totalsize);
    }
    void CPUStream::Write(void * outData) {
	memcpy(outData,data,stride*totalsize);
    }
    void CPUStream::Release() {
	delete this;
    }
    CPUStream::~CPUStream() {
	free(data);
	free(extents);
    }
       
}
