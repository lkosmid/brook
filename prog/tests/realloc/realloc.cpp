#include <brook/brook.hpp>
#include "realloc.h"

void streamRealloc1D (::brook::stream &s, int n) {
  ::brook::Stream *S = s;
  ::brook::StreamType type[100];
  unsigned int i;

  for (i=0; i<S->getFieldCount(); i++)
    type[0] = S->getIndexedFieldType(i);
  type[i] = ::brook::__BRTNONE;

  s = ::brook::stream(&(type[0]), n,-1);
}

void streamRealloc2D (::brook::stream &s, int y, int x) {
  ::brook::Stream *S = s;
  ::brook::StreamType type[100];
  unsigned int i;

  for (i=0; i<S->getFieldCount(); i++)
    type[0] = S->getIndexedFieldType(i);
  type[i] = ::brook::__BRTNONE;

  s = ::brook::stream(&(type[0]), y, x,-1);
}





