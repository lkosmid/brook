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
static map<string,int> getKnownTypes() {
    map<string,int>ret;
    ret.insert(mapstrint::value_type("float",sizeof(float)));
    ret.insert(mapstrint::value_type("float1",sizeof(float)));
    ret.insert(mapstrint::value_type("float2",sizeof(float2)));
    ret.insert(mapstrint::value_type("float3",sizeof(float3)));
    ret.insert(mapstrint::value_type("float4",sizeof(float4)));            
    ret.insert(mapstrint::value_type("int",sizeof(int)));
    ret.insert(mapstrint::value_type("int1",sizeof(int)));
    ret.insert(mapstrint::value_type("int2",2*sizeof(int)));
    ret.insert(mapstrint::value_type("int3",3*sizeof(int)));
    ret.insert(mapstrint::value_type("int4",4*sizeof(int)));            
    ret.insert(mapstrint::value_type("char",sizeof(char)));
    ret.insert(mapstrint::value_type("char1",sizeof(char)));
    ret.insert(mapstrint::value_type("char2",2*sizeof(char)));
    ret.insert(mapstrint::value_type("char3",3*sizeof(char)));
    ret.insert(mapstrint::value_type("char4",4*sizeof(char)));            
    return ret;
}
static mapstrint knownTypes = getKnownTypes();

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
int knownTypeSize (__BRTStreamType type) {
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
    CPUStream::CPUStream(__BRTStreamType type, int dims, int extents[]){
	this->extents = (unsigned int *)malloc(dims*sizeof(unsigned int));
	totalsize=1;
	for(int i=0;i<dims;++i) {
	    this->extents[i]=extents[i];
	    totalsize*=extents[i];
	}
        this->type = type;
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
