/* $Id$ */

#include "ppmImage.h"
#include <stdio.h>

ppmImage::ppmImage( int img_width, int img_height ){
  width = img_width;
  height = img_height;
  int size = width*height*4;
  data = new float[size];
  for(int i=0; i<size; i++)
    data[i] = 0.0f;
}

void ppmImage::AddSample( int x, int y, const Spectra& color, const Spectra& weight ){
  Spectra contrib = color*weight;
  int index = (y*width+x)*4;
  data[index+0] += contrib.R();
  data[index+1] += contrib.G();
  data[index+2] += contrib.B();
  data[index+3] += 1.0f;
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
