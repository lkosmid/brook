#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#define STRIP_16 true

static void png_cexcept_error(png_structp png_ptr, png_const_charp msg) {
	if(png_ptr) {
		// Do nothing?
	}
	fprintf(stderr, "Libpng error: %s\n", msg);
}

int readPng(const char *filename, unsigned char **data, unsigned int *width, unsigned int *height, int *color_type, int *bpp) {
	FILE * fp = fopen (filename, "rb");
	if (!fp) {
		fprintf(stderr,"Unable to open file \"%s\" for reading.\n",filename);
		*data=NULL;
		*width=*height=*color_type=*bpp=0;
		return 0;
	}
	unsigned char sig[8];
	png_structp png_ptr;
	png_bytepp row_pointers;
	png_infop info_ptr;
	int  interlace_type;
	fread(sig, 1, 8, fp);
	if (!png_check_sig(sig, 8)) {
		fprintf(stderr, "Invalid PNG image \"%s\".\n",filename);
		*data=NULL;
		*width=*height=*color_type=*bpp=0;
		return 0;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
	(png_error_ptr)png_cexcept_error, 
	(png_error_ptr)NULL);
	if (png_ptr == NULL) {
		fprintf(stderr,"Error in png_create_read_struct() while reading image \"%s\".\n",filename);
		*data=NULL;
		*width=*height=*color_type=*bpp=0;
		return 0;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fprintf(stderr,"Error in png_create_info_struct() while reading image \"%s\".\n",filename);
		*data=NULL;
		*width=*height=*color_type=*bpp=0;
		return 0;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		fprintf(stderr,"Error in setjmp() while reading image \"%s\".\n",filename);
		*data=NULL;
		*width=*height=*color_type=*bpp=0;
		return 0;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
#ifdef PNGDEBUG
	printf ("Loading Done. Decompressing\n");
#endif
	png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)width, (png_uint_32 *)height, bpp, color_type, &interlace_type, NULL, NULL);
# if __BYTE_ORDER != __BIG_ENDIAN
	if (*bpp==16)
		png_set_swap (png_ptr);
#endif
	if (*bpp==16&&STRIP_16)
		png_set_strip_16(png_ptr);
	if (STRIP_16&&*color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY && *bpp < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	png_set_expand (png_ptr);
	png_read_update_info (png_ptr,info_ptr);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)width, (png_uint_32 *)height, bpp, color_type, &interlace_type, NULL, NULL);
	row_pointers = (unsigned char **)malloc (sizeof(unsigned char *) * (*height));
	int numchan=1;
	if (*color_type&PNG_COLOR_MASK_COLOR)
		numchan =3;
	if (*color_type &PNG_COLOR_MASK_PALETTE)
		numchan =1;
	if (*color_type&PNG_COLOR_MASK_ALPHA)
		numchan++;
	unsigned long stride = numchan*sizeof (unsigned char)*(*bpp)/8;
	unsigned char * image = (unsigned char *) malloc (stride*(*width)*(*height));
	for (unsigned int i=0;i<*height;i++) {
		row_pointers[*height-i-1] = &image[i*stride*(*width)];
	}
	png_read_image (png_ptr,row_pointers);
	//   png_read_image(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND , NULL);
	//row_pointers = png_get_rows(png_ptr, info_ptr);
	*data = image;
	free (row_pointers);
	//   png_infop end_info;
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#ifdef PNGDEBUG
	printf ("Decompressing Done.\n");
#endif
	fclose (fp);
	return 1;
}

int writePng(const char *filename, const unsigned char *data, unsigned int width, unsigned int height, int color_type, int bpp) {

	FILE * fp = fopen (filename, "wb");
        int channels=1;
        switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:
        case PNG_COLOR_TYPE_PALETTE:
           channels=1;      
           break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
           channels=4;
           break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
           channels=2;
           break;
        case PNG_COLOR_TYPE_RGB:
        default:
           channels=3;
        }
        int stride = channels * bpp/8;
	if (!fp) {
		fprintf(stderr,"Unable to open file \"%s\"for writing.\n",filename);
		return 0;
	}
	png_structp png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		fprintf(stderr,"Error in png_create_info_struct() while writing image \"%s\".\n",filename);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 0;
	}
	if (setjmp(png_ptr->jmpbuf)) {
		fprintf(stderr,"Error in setjmp() while writing image \"%s\".\n",filename);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 0;
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
		bpp, 
		color_type, 
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, 
		PNG_FILTER_TYPE_DEFAULT);
	{
		png_write_info(png_ptr, info_ptr);
		png_byte **row_pointers;
		row_pointers= (png_byte**)malloc(height*sizeof(png_byte*));
		for (unsigned int i=0;i<height;i++) {
			row_pointers[i]= (png_byte *)&data[i*width*stride];
		}
		png_write_image (png_ptr,row_pointers);
		png_write_end(png_ptr, info_ptr);
		png_write_flush(png_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free (row_pointers);
	}
	fclose (fp);
	return 1;
}

