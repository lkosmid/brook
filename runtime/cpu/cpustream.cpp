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
    ret.insert(map<string,int>::value_type("char",sizeof(char)));
    ret.insert(map<string,int>::value_type("char1",sizeof(char)));
    ret.insert(map<string,int>::value_type("char2",2*sizeof(char)));
    ret.insert(map<string,int>::value_type("char3",3*sizeof(char)));
    ret.insert(map<string,int>::value_type("char4",4*sizeof(char)));            
    return ret;
}
static map<string,int> knownTypes = getKnownTypes();

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
int knownType (std::string typ) {
	return knownTypes[noWhiteSpace(typ)];
}
namespace brook{
    CPUStream::CPUStream(const char type[], int dims, int extents[]){
	this->extents = (unsigned int *)malloc(dims*sizeof(unsigned int));
	totalsize=1;
	for(int i=0;i<dims;++i) {
	    this->extents[i]=extents[i];
	    totalsize*=extents[i];
	}
	stride=knownType(type);
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
