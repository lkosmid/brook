/* $Id: ppmImage.h,v 1.1 2004/03/23 20:39:03 danielrh Exp $ */

#ifndef PPMIMAGE_H
#define PPMIMAGE_H

class ppmImage {
public:
  ppmImage( int img_width, int img_height );
  ~ppmImage();
  int Width() const { return width; }
  int Height() const { return height; }
  void Write( char *fname );
  float *Data() { return data; }
  void Clear() { for(int i=0; i<width*height*4; i++) data[i] = 0.0f; }
private:
  int width;
  int height;
  float *data;
};

#endif /* PPMIMAGE_H */
