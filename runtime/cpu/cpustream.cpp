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


namespace brook{
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    CPUStream::CPUStream(  int inFieldCount, 
                           const __BRTStreamType* inFieldTypes,
                           int dims,
                           const int extents[]){
       elementType.insert(elementType.begin(),inFieldTypes,inFieldTypes+inFieldCount);
	this->extents = (unsigned int *)malloc(dims*sizeof(unsigned int));
        this->dims = dims;
	totalsize=1;
	for(int i=0;i<dims;++i) {
	    this->extents[i]=extents[i];
	    totalsize*=extents[i];
	}
	stride=getElementSize();
	if (stride) {
	    data = malloc(stride*totalsize);
            //std::cerr << "Mallocing "<<stride<< "*"<< totalsize<<std::endl;;
	}else {
	    std::cerr<<"Failure to produce stream: 0 types."<<std::endl;
	}
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUStream::Read(const void*inData) {
	memcpy(data,inData,stride*totalsize);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUStream::Write(void * outData) {
	memcpy(outData,data,stride*totalsize);
    }

   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    void CPUStream::Release() {
	delete this;
    }
   
   // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
    CPUStream::~CPUStream() {
	free(data);
	free(extents);
    }
       
}
