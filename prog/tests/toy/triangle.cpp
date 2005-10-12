#include <GL/glut.h>
#include <brook/brook.hpp>
#include <GL/glext.h>
int totwidth,totheight;
extern int texturewidth,textureheight;
void reshape (int width, int height) {
  glViewport(0,0,width,height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width,0,height,-1,1);
  totwidth=width;
  totheight=height;
}
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
GLuint my_texture_names=0;
int dotex() {
  glGenTextures(1, &my_texture_names);  
  char * my_texture_scratch=(char*)malloc(4*textureheight*texturewidth);
  memset(my_texture_scratch,0x1c,4*textureheight*texturewidth);
  for (int i=0;i<textureheight*texturewidth;++i) {
    my_texture_scratch[i*4]=i;
    my_texture_scratch[i*4+1]=i;
    my_texture_scratch[i*4+2]=i;
  }
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, my_texture_names);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST);



  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, textureheight,
               texturewidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, my_texture_scratch);

  glEnable(GL_TEXTURE_RECTANGLE_ARB);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  return my_texture_names;
}
extern int maino();
void display () {
  glClear(GL_COLOR_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, my_texture_names);
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex2i(0, 0);
  glTexCoord2f(texturewidth,0);
  glVertex2i(totwidth, 0);
  glTexCoord2f(texturewidth,textureheight);
  glVertex2i(totwidth, totheight);
  glTexCoord2f(0,textureheight);
  glVertex2i(0, totheight);
  glEnd();
  glFlush();
  
  //  brook::initialize("ogl");
  //  maino();

}
int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutCreateWindow("Box");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  int name=dotex();

  glutMainLoop();
  return 0;    
}
