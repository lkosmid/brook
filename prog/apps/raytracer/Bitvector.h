#ifndef BITVECTOR_H
#define BITVECTOR_H

#include "Types.h"
#include <stdio.h>
#include <stdlib.h>

#define WORDSIZE 64

class BitVector{
public:
  BitVector( const Tuple3i& dimensions, int C ) {
    dims = dimensions;
    dims.x += (C-dims.x%C);
    dims.y += (C-dims.y%C);
    dims.z += (C-dims.z%C);
    num_entries = dims.x*dims.y*dims.z;
    num_elements = num_entries/WORDSIZE + 1;
    if(num_entries <= 0){
      fprintf(stderr, "Bad BVector size: %ix%ix%i\n", dims.x, dims.y, dims.z);
      exit(0);
    }
    elements = new u64[num_elements];
    for(int i=0; i<num_elements; i++)
      elements[i] = (u64)0;

    //initialize LUTs
    G = new int[dims.x];
    H = new int[dims.y];
    I = new int[dims.z];
    for(int u=0; u<dims.x; u++)
      G[u] = (u/C)*C*C*C + u%C;
    for(int v=0; v<dims.y; v++)
      H[v] = (v/C)*(dims.x/C)*C*C*C + (v%C)*C;
    for(int w=0; w<dims.z; w++)
      I[w] = (w/C)*(dims.x/C)*(dims.y/C)*C*C*C + (w%C)*C*C;
  }

  ~BitVector(){
    delete [] elements;
    delete [] G;
    delete [] H;
    delete [] I;
  }

  void Set(int u, int v, int w) {
    int index = G[u]+H[v]+I[w];
    int position = index / WORDSIZE;
    int offset = index % WORDSIZE;
    u64 value = (u64)1 << offset;
    elements[position] = elements[position] | value;
  }

  bool Check(int u, int v, int w){
    //#ifdef STREAM
    bool validindex = u < dims.x && v < dims.y && w < dims.z;
    int index = validindex ? G[u]+H[v]+I[w] : 0;
    int position = index / WORDSIZE;
    int offset = index % WORDSIZE;
    u64 value = (u64)1 << offset;
    return ( validindex && (elements[position] & value) != (u64)0 );
    //#else
    //int index = G[u]+H[v]+I[w];
    //int position = index / WORDSIZE;
    //int offset = index % WORDSIZE;
    //u64 value = (u64)1 << offset;
    //return ( (elements[position] & value) != (u64)0 );
    //#endif
  }

  int GetPosition(int u, int v, int w){
    int index = G[u]+H[v]+I[w];
    int position = index / WORDSIZE;
    return position;
  }

  u64 GetWord(int u, int v, int w){
    int index = G[u]+H[v]+I[w];
    int position = index / WORDSIZE;
    return elements[position];
  }

  void DumpList(){
    for(int i=0; i<num_elements; i++){
      if(elements[i])
	fprintf(stderr, "Element %i has data\n", i);
      else
	fprintf(stderr, "Element %i is empty\n", i);
    }
  }

  void WriteToFile(FILE *fp){
    fwrite( elements, sizeof(u64), num_elements, fp );
  }

  void ReadFromFile(FILE *fp){
    fread( elements, sizeof(u64), num_elements, fp );
  }
  
private:
  u64 *elements;
  int num_elements;
  int num_entries;
  //lookup tables
  int *G, *H, *I;
  Tuple3i dims;
};

#endif /* BITVECTOR_H */
