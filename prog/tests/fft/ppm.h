int readPPM(const char *filename, 
	    unsigned char **data, 
	    unsigned int *width, 
	    unsigned int *height) {
  char header[256];
  int nbyte;

  FILE *fp = fopen(filename, "rb");
  assert (fp);

  fscanf (fp, "%s", header);
  assert (!strncmp(header, "P6", strlen("P6")));

  fscanf (fp, "%d %d %d\n", width, height, &nbyte);
  assert (nbyte == 255);
  assert (*width > 0 && *width <= 2048);
  assert (*height > 0 && *height <= 2048);

  *data = (unsigned char *) malloc (*width * *height * 3);
  assert (*data);
  fread(*data, *width * *height, 3, fp);
  fclose(fp);

  return 0;
}


int writePPM(const char *filename, 
	     const unsigned char *data, 
	     unsigned int width, 
	     unsigned int height) {

  FILE *fp = fopen(filename, "wb");
  assert (fp);
   
  fprintf(fp, "%s\n%d %d %d\n", "P6", width, height, 255);
  fwrite(data, width*height, 3, fp);
  fclose(fp);

  return 0;
}


