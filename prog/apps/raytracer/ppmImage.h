/* $Id$ */

#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include "Types.h"

class ppmImage {
public:
  ppmImage( int img_width, int img_height );
  ~ppmImage() { delete [] data; }
  int Width() const { return width; }
  int Height() const { return height; }
  void AddSample( int x, int y, const Spectra& color, const Spectra& weight );
  void Write( char *fname );
  float *Data() { return data; }
  void Clear() { for(int i=0; i<width*height*4; i++) data[i] = 0.0f; }
private:
  int width;
  int height;
  float *data;
};

#endif /* PPMIMAGE_H */
