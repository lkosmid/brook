#include <stdio.h>
#include <stdlib.h>
#ifdef USE_PNG
#include "png.h"
#endif
void writePng (char * filename, char * data, int width,int height) {
    {
      FILE * fp = fopen (filename, "wb");
#ifndef USE_PNG
      char header[1024];
      sprintf(header,"P6\n%d %d 255\n",width,height);
      char * hptr=&header[0];
      while (*hptr) {
         fputc(*hptr++,fp);
      }
      for (int i=0;i<width*height;++i) {
         fputc(data[i],fp);
         fputc(data[i],fp);
         fputc(data[i],fp);
      }
#else
      png_structp png_ptr = png_create_write_struct
        (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return;
      }
      if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
      }
      png_init_io(png_ptr, fp);
      png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
      png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
      /* set other zlib parameters */
      png_set_compression_mem_level(png_ptr, 8);
      png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
      png_set_compression_window_bits(png_ptr, 15);
      png_set_compression_method(png_ptr, 8);  
      png_set_IHDR(png_ptr, 
                   info_ptr, 
                   width,
                   height,
                   8, 
                   PNG_COLOR_TYPE_GRAY, 
                   PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT, 
                   PNG_FILTER_TYPE_DEFAULT);  
      {
        png_write_info(png_ptr, info_ptr);
        png_byte **row_pointers;
        row_pointers= (png_byte**)malloc(height*sizeof(png_byte*));
        for (int i=0;i<height;i++) {
          row_pointers[i]= (png_byte *)&data[i*width];
        }
        png_write_image (png_ptr,row_pointers);
        png_write_end(png_ptr, info_ptr);
        png_write_flush(png_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        free (row_pointers);
      }
#endif
      fclose (fp);
    }
}
