#include <vector>

struct den_header {
   unsigned char  map_max[62];//44,45 && 46,47 && 48,49
};
struct ppm {
   FILE * fp;
   unsigned int start;
   unsigned int width;
   unsigned int height;
   unsigned int depth;
   unsigned int numspheres;
   float spheredist;
   std::vector<float3> vertices;
   ppm () {numspheres=1;spheredist=2.0f;fp=NULL;start=width=height=0;}
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
   ppm ret;
   ret.fp = fopen (name,"rb");
   if (ret.fp) {
      ret.width=ret.height=ret.depth=256;
      den_header dh;
      fread(&dh,sizeof(char)*62,1,ret.fp);
      ret.width=((int)dh.map_max[45])+256*(int)dh.map_max[44];
      ret.height=((int)dh.map_max[47])+256*(int)dh.map_max[46];
      ret.depth=((int)dh.map_max[49])+256*(int)dh.map_max[48];
      ret.start = ftell(ret.fp);
   }
   return ret;
}
void printVolume (ppm &fp) {
   std::vector<float3>::const_iterator i=fp.vertices.begin();
   unsigned int j=0;
   static char jj = '1';
   char nam[]="output1.xmesh";
   nam[6]=jj++;
   FILE * f = fopen(nam,"w");
   fprintf (f,"<Mesh texture=\"white.bmp\"><material cullface=\"false\"/>\n");
   fprintf(f,"<Points>\n");
   for (;i!=fp.vertices.end();++i) {
      if (finite_float(i->x)&&finite_float(i->y)&&finite_float(i->z)) {
         if (i->x>-.50&&i->y>-.50&&i->z>-.50) {
           if (i->x!=36893206672442393000.00) {
              fprintf (f,"<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/><Normal i=\"1\" j=\"0\" k=\"0\"/></Point>\n",i->x,i->y,i->z);
             //            printf ("<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/></Point>\n",i->x+.5,i->y+.5,i->z+.5);
             j++;
           }
         }
      }
   }
   fprintf(f,"</Points>\n<Polygons>\n");
   {for (unsigned int i=0;i<j/3;++i) {
         fprintf (f,"<Tri> <Vertex point=\"%d\"/><Vertex point=\"%d\"/><Vertex point=\"%d\"/></Tri>\n",
              i*3,i*3+1,i*3+2);
      
   }}
   fprintf (f,"</Polygons></Mesh>\n");
   fp.vertices.clear();
   fclose(f);
}
void printVolume (const ppm &fp) {
   std::vector<float3>::const_iterator i=fp.vertices.begin();
   unsigned int j=0;
   printf ("<Mesh texture=\"white.bmp\"><material cullface=\"false\"/>\n");
   printf("<Points>\n");
   for (;i!=fp.vertices.end();++i) {
      if (finite_float(i->x)&&finite_float(i->y)&&finite_float(i->z)) {
         if (i->x>-.50&&i->y>-.50&&i->z>-.50) {
           if (i->x!=36893206672442393000.00) {
             printf ("<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/><Normal i=\"1\" j=\"0\" k=\"0\"/></Point>\n",i->x,i->y,i->z);
             //            printf ("<Point><Location x=\"%3.2f\" y=\"%3.2f\" z=\"%3.2f\"/></Point>\n",i->x+.5,i->y+.5,i->z+.5);
             j++;
           }
         }
      }
   }
   printf("</Points>\n<Polygons>\n");
   {for (unsigned int i=0;i<j/3;++i) {      printf ("<Tri> <Vertex point=\"%d\"/><Vertex point=\"%d\"/><Vertex point=\"%d\"/></Tri>\n",
              i*3,i*3+1,i*3+2);
      
   }}
   printf ("</Polygons></Mesh>\n");
}
float * mallocSlice (const ppm &fp) {
   return (float*)malloc(sizeof(float)*fp.width*fp.height);
}
float myrand () {
  return rand()>RAND_MAX/2?1.0f:0.0f;
   static unsigned int seed1=21051095;
   unsigned int * seed=&seed1;
   unsigned int rand_max =1509281;
   *seed = (*seed +26129357)%rand_max;
   if (*seed<(rand_max/2))
      return 1;
   else return -1;
}

void readPPM3dSlice(const ppm &fp, 
                   unsigned int whichslice,
                   float *data) {
   unsigned int size = fp.width*fp.height;
   static bool dorandom=false;
   if (fp.fp) {
      char * readindata = (char *) data;
      fseek (fp.fp,fp.start+whichslice*size*sizeof(char),SEEK_SET);
      assert (sizeof(float)==4*sizeof(char));
      readindata+=size+size+size;
      fread(readindata, size, 1, fp.fp);
      for (unsigned int i=0;i<size;++i) {
         data[i]=readindata[i]/255.0f-.5f;//because we only support float format!
      }
   }else if (dorandom)for (unsigned int i=0;i<size;++i) data[i] = myrand();
   else {
     float trad = (float)(fp.width-1)/2;
     float offset =(float) (fp.width-1)/2;
     for (unsigned int j=0;j<fp.height;++j) {
       for (unsigned int i=0;i<fp.width;++i) {
         if (fp.width<=3)
           data[i+j*fp.width]= (i==1&&j==1&&whichslice==1)?1.0f:0.0f;
         else {
            bool mybool=false;
            bool alt = true;
            float rad = trad;
            for (unsigned int l=0;l<fp.numspheres;++l) {
              
              float newj = (j-offset);
              float newi = (i-offset);
              float newk = (whichslice-offset);
              float loc = newj*(float)newj+newk*(float)newk+newi*(float)newi;
              if (loc<rad*rad) {
                mybool = alt;
                alt = !alt;
              }else {
                break;
              }
              rad-=fp.spheredist;
            }
            data[i+j*fp.width]=mybool?1.0f:0.0f;

         }
       }
     }
   }
}
void closePPM (const ppm &fp) {
   fclose (fp.fp);
}
unsigned int findNaN(std::vector<float3> v) {
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
float3* consolidateVertices(ppm &fp,float4 ss/*stream size*/) {
   unsigned int siz = fp.vertices.size();
   fp.vertices.insert(fp.vertices.end(),
                      (unsigned int)ss.x*(unsigned int)ss.y,
                      float3(0,0,0));
   return &fp.vertices[siz];
   unsigned int size = (unsigned int)ss.x*(unsigned int)ss.y;
   unsigned int nanloc= findNaN(fp.vertices);
   if (size>fp.vertices.size()-nanloc) {
      unsigned int newguys = size-(fp.vertices.size()-nanloc);
      fp.vertices.insert(fp.vertices.end(),newguys,float3(0,0,0));
   }
   if (!&fp.vertices[nanloc]) {
      static float3 x;
      return &x;
   }
   return &fp.vertices[nanloc];
}
