#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif
#include "cpu.hpp"
#include <map>
#include <string>
#include <iostream>
using std::map;
using std::string;
typedef std::map<std::string,int> mapstrint;

std::string noWhiteSpace (std::string in) {
	unsigned int j=0;
	for (unsigned int i=0;i<in.length();++i) {
		in[j]=in[i];
		if (in[i]==' '||in[i]=='\t'||in[i]=='\n'||in[i]=='\r')
			;
		else
			j++;
	}
	return in.substr(0,j);												
}
unsigned int knownTypeSize (__BRTStreamType type) {
   switch (type) {
   case __BRTFLOAT2:
      return sizeof(float2);
   case __BRTFLOAT3:
      return sizeof(float3);
   case __BRTFLOAT4:
      return sizeof(float4);
   default:
      return sizeof(float);
   }
}
namespace brook{
    CPUStream::CPUStream(__BRTStreamType type, int dims, int extents[]):Stream(type){
       
	this->extents = (unsigned int *)malloc(dims*sizeof(unsigned int));
        this->dims = dims;
	totalsize=1;
	for(int i=0;i<dims;++i) {
	    this->extents[i]=extents[i];
	    totalsize*=extents[i];
	}
	stride=knownTypeSize(type);
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
