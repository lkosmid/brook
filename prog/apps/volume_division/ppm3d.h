#include <vector>
struct ppm {
   FILE * fp;
   unsigned int start;
   unsigned int width;
   unsigned int height;
   unsigned int depth;
   std::vector<float4> vertices;
   ppm () {fp=NULL;start=width=height=0;}
   void init (unsigned int width,unsigned int height,unsigned int depth) {
      this->width=width;
      this->height=height;
      this->depth=depth;
   }
};
ppm randomPPM (unsigned int width,unsigned int height, unsigned int depth) {
   ppm p;
   p.init (width,height,depth);
   return p;
}
ppm openPPM (char * name) {
   int nbyte;
   ppm ret;
   ret.fp = fopen (name,"rb");
   if (ret.fp) {
      ret.width=ret.height=ret.depth=256;
      fscanf (ret.fp, "P5\n %d %d %d\n", &ret.width, &ret.depth, &nbyte);
      ret.height=ret.width;
      ret.start = ftell(ret.fp);
   }
   return ret;
}
void printVolume (const ppm &fp) {
   std::vector<float4>::const_iterator i=fp.vertices.begin();
   unsigned int j=0;
   printf ("<Mesh texture=\"white.bmp\">\n");
   printf("<Points>\n");
   for (;i!=fp.vertices.end();++i) {
      if (finite_float(i->x)&&finite_float(i->y)&&finite_float(i->z)) {
         if (i->x>-.50&&i->y>-.50&&i->z>-.50) {
           if (i->x!=36893206672442393000.00) {
             printf ("<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/><Normal i=\"1\" j=\"0\" k=\"0\"/></Point>\n",i->x,i->y,i->z);
             printf ("<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/></Point>\n",i->x+.5,i->y+.5,i->z+.5);
             ++j;
           }
         }
      }
   }
   printf("</Points>\n<Polygons>\n");
   {for (unsigned int i=0;i<j;++i) {
      printf ("<Line> <Vertex point=\"%d\"/><Vertex point=\"%d\"/></Line>\n",
              i*2,i*2+1);
      
   }}
   printf ("</Polygons></Mesh>\n");
}
float * mallocSlice (const ppm &fp) {
   return (float*)malloc(sizeof(float)*fp.width*fp.height);
}
void readPPM3dSlice(const ppm &fp, 
                   unsigned int whichslice,
                   float *data) {
   unsigned int size = fp.width*fp.height;
   if (fp.fp) {
      char * readindata = (char *) data;
      fseek (fp.fp,fp.start+whichslice*size*sizeof(char),SEEK_SET);
      assert (sizeof(float)==4*sizeof(char));
      readindata+=size+size+size;
      fread(readindata, size, 1, fp.fp);
      for (unsigned int i=0;i<size;++i) {
         data[i]=readindata[i]/255.0f-.5f;//because we only support float format!
      }
   }else for (unsigned int i=0;i<size;++i) data[i] = ((float)rand())/RAND_MAX-.5f;
}
void closePPM (const ppm &fp) {
   fclose (fp.fp);
}
unsigned int findNaN(std::vector<float4> v) {
   unsigned int half = v.size()/4;
   unsigned int pos = v.size()/2;
   while (half) {
      if (isinf_float(v[pos].x)==0) {
         pos+=half;
      }else {
         if (isinf_float(v[pos-half].x)) {
            pos-=half;
         }
      }
      half/=2;
   }
   if (pos<v.size())
      if (!isinf_float(v[pos].x))
         pos+=1;
   return pos;
}
float4* consolidateVertices(ppm &fp,float4 ss/*stream size*/) {
   unsigned int siz = fp.vertices.size();
   fp.vertices.insert(fp.vertices.end(),
                      (unsigned int)ss.x*(unsigned int)ss.y,
                      float4(0.0f,0.0f,0.0f,0.0f));
   return &fp.vertices[siz];
   unsigned int size = (unsigned int)ss.x*(unsigned int)ss.y;
   unsigned int nanloc= findNaN(fp.vertices);
   if (size>fp.vertices.size()-nanloc) {
      unsigned int newguys = size-(fp.vertices.size()-nanloc);
      fp.vertices.insert(fp.vertices.end(),newguys,float4(0,0,0,0));
   }
   if (!&fp.vertices[nanloc]) {
      static float4 x;
      return &x;
   }
   return &fp.vertices[nanloc];
}
