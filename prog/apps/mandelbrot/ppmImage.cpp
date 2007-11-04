/* $Id: ppmImage.cpp,v 1.1 2004/03/23 20:39:03 danielrh Exp $ */

#include "ppmImage.h"
#include "brook/brook.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

ppmImage::ppmImage( int img_width, int img_height ){
  width = img_width;
  height = img_height;
  int size = width*height*4;
  data = (float *) brmalloc(size*sizeof(float));
  assert(data);
  for(int i=0; i<size; i++)
    data[i] = 0.0f;
}

ppmImage::~ppmImage(){
  brfree(data);
}

void ppmImage::Write( char *fname ){
  FILE *fp = fopen( fname, "wb" );
  if (!fp)
    fprintf(stderr, "Couldn't open PPM file for writing\n");
  else{
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", width, height );
    fprintf(fp, "255\n" );
    for(int i=0; i<width*height*4; i+=4){
      fputc( (char)(data[i+0]*255.0), fp );
      fputc( (char)(data[i+1]*255.0), fp );
      fputc( (char)(data[i+2]*255.0), fp );
    }
    fclose(fp);
  }
}
