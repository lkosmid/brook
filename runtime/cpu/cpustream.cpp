
#ifdef _WIN32
#pragma warning(disable:4786)
//  the above warning disables visual studio's 
//  annoying habit of warning when using the standard set lib
#endif

#include "cpu.hpp"
#include <map>
#include <string>
#include <iostream>
#include <math.h>
#include <assert.h>

using std::map;
using std::string;

namespace brook{

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  CPUStream::CPUStream(  unsigned int inFieldCount, 
                         const StreamType* inFieldTypes,
                         unsigned int dims,
                         const unsigned int extents[]){

    unsigned int i;

    elementType.insert(elementType.begin(),
                       inFieldTypes,
                       inFieldTypes+inFieldCount);

    this->extents    = (unsigned int *) malloc (dims*sizeof(unsigned int));
    this->domain_min = (unsigned int *) malloc (dims*sizeof(unsigned int));
    this->domain_max = (unsigned int *) malloc (dims*sizeof(unsigned int));
    this->pos        = (unsigned int *) malloc (dims*sizeof(unsigned int));

    this->dims = dims;
    totalsize=1;

    for(i=0; i<dims; ++i) {
      this->extents[i]    = extents[i];
      this->domain_min[i] = 0;
      this->domain_max[i] = extents[i];

      totalsize *= extents[i];
    }

    stride=getElementSize();

	fprintf (stderr, "******************************\n");
	fprintf (stderr, "mallocing: %d * %d\n", stride, totalsize);
	fflush (stderr);

    if (stride) {
      data = malloc(stride*totalsize);
    } else {
      std::cerr<<"Failure to produce stream: 0 types."<<std::endl;
    }

    malloced_size = stride*totalsize;

	fprintf (stderr, "******************************\n\n");
	fflush (stderr);

    isDerived = false;
  }
  
  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  CPUStream::CPUStream(const CPUStream &a,
                       const int *min,
                       const int *max) {
    unsigned int i;

    elementType = a.elementType;
    data        = a.data;
    extents     = a.extents;
    dims        = a.dims;
    stride      = a.stride;
    totalsize   = a.totalsize;
    

	fprintf (stderr, "******************************\n");
	fprintf (stderr, "Derived mallocing\n");
	fflush  (stderr);

    pos         = (unsigned int *) malloc (sizeof(unsigned int) * dims);
    domain_min  = (unsigned int *) malloc (sizeof(int) * dims);
    domain_max  = (unsigned int *) malloc (sizeof(int) * dims);

    for (i=0; i<dims; i++) {
      domain_min[i] = a.domain_min[i] + min[i];
      domain_max[i] = a.domain_min[i] + max[i]; 
    }

	fprintf (stderr, "******************************\n");

    isDerived = true;
  }


  CPUStreamShadow::CPUStreamShadow(StreamInterface *s,
                                   unsigned int flags) {
    unsigned int i;

    const unsigned int *e    = s->getExtents();
    const unsigned int *dmin = s->getDomainMin();
    const unsigned int *dmax = s->getDomainMax();

    for (i=0; i<s->getFieldCount(); i++)
      elementType.push_back(s->getIndexedFieldType(i));

    data      = s->getData(flags);
    dims      = s->getDimension();
    stride    = s->getElementSize();
    totalsize = s->getTotalSize();
    
	fprintf (stderr, "\n\n Shadow Stream\n");
	fflush (stderr);

	extents    = (unsigned int *) malloc (dims*sizeof(unsigned int));
    domain_min = (unsigned int *) malloc (dims*sizeof(unsigned int));
    domain_max = (unsigned int *) malloc (dims*sizeof(unsigned int));
    pos        = (unsigned int *) malloc (dims*sizeof(unsigned int));

    for (i=0; i<dims; i++) {
      extents[i]    = e[i];
      domain_min[i] = dmin[i];
      domain_max[i] = dmax[i];
    }

    shadow = s;
    this->flags = flags;

	isDerived = true;
  }

  CPUStreamShadow::~CPUStreamShadow() {

	fprintf (stderr, "\n\n 0x%p: Stream Shadow Destructor\n", this);
	fflush (stderr);
	
#if 0 
	assert(extents);
    free(extents);
	extents = NULL;

	assert(pos);
	free(pos);
	pos = NULL;
	
    assert (domain_min);
	free(domain_min);
	domain_min = NULL;

    assert(domain_max);
    free(domain_max);
    domain_max = NULL;
#endif
    
	shadow->releaseData(flags);
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
  Stream * CPUStream::Domain(int min, int max) {
    assert (dims == 1);
    return new CPUStream(*this, &min, &max);
  } 

  Stream * CPUStream::Domain(const int2 &min, const int2 &max) {
    assert (dims == 2);
    return new CPUStream(*this, (const int *) &min, (const int *) &max);
  }

  Stream * CPUStream::Domain(const int3 &min, const int3 &max) {
    assert (dims == 3);
    return new CPUStream(*this, (const int *) &min, (const int *) &max);
  }

  Stream * CPUStream::Domain(const int4 &min, const int4 &max) {
    assert (dims == 4);
    return new CPUStream(*this, (const int *) &min, (const int *) &max);
  }
  
  void * CPUStream::fetchElem(const unsigned int curpos[], 
                              const unsigned int bounds[],
                              unsigned int kdim) {
    unsigned int i;
    for (i=0; i<dims; i++)
      if (i < kdim)
        pos[i] = curpos[i] * (domain_max[i] - domain_min[i]) / bounds[i];
      else
        pos[i] = 0;
  
    /* Fetch element from ptr */
    return fetchItemPtr(data, pos);
  }
     

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  CPUStream::~CPUStream() {

	fprintf (stderr, "\n\n 0x%p: CPU Stream Destructor\n", this);

    assert (domain_min);
	free(domain_min);
	domain_min = NULL;

    assert(domain_max);
    free(domain_max);
    domain_max = NULL;

	assert(pos);
	free(pos);
	pos = NULL;

    if (!isDerived) {
	  assert(extents);
      free(extents);
	  extents = NULL;

	  assert(data);
      free(data);
	  data = NULL;
    }
  }   
}
