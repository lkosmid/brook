#include <stdio.h>
#include "FileIO.h"
#include "HMatrix3.h"
#include "ply.h"


typedef struct Vertex {
  float x,y,z;                     /* the usual 3-space position of a vertex */
  float nx,ny,nz;                  /* vertex normal components */
  unsigned char diffuse_red,
                diffuse_green,
                diffuse_blue;      /* vertex color components */
  void* other_props;
} Vertex;

typedef struct Face {
  unsigned char nverts;           /* number of vertex indices in list */
  int *verts;                     /* vertex index list */
  void* other_props;
} Face;


/* The kinds of elements in the user's object */
static char *elem_names[] = {
  "vertex", "face"
};

/* List of property information for a vertex */
static PlyProperty vert_props[] = {
  {"x", Float32, Float32, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(Vertex,nz), 0, 0, 0, 0},
  {"diffuse_red", Uint8, Uint8, offsetof(Vertex,diffuse_red), 0, 0, 0, 0},
  {"diffuse_green", Uint8, Uint8, offsetof(Vertex,diffuse_green), 0, 0, 0, 0},
  {"diffuse_blue", Uint8, Uint8, offsetof(Vertex,diffuse_blue), 0, 0, 0, 0},
};

/* List of property information for a face */
static PlyProperty face_props[] = {
  { "vertex_indices", Int32, Int32, offsetof(Face,verts),
    1, Uint8, Uint8, offsetof(Face,nverts) },
};


void ReadPlyFile( char *filename, int &numtris, HMatrix3 xform,
		  Point3* &v0, Point3* &v1, Point3* &v2,
		  Normal3* &n0, Normal3* &n1, Normal3* &n2,
		  Spectra* &c0, Spectra* &c1, Spectra* &c2){
  fprintf(stderr, "\tReading %s... ", filename);
  PlyFile *in_ply;
  int nverts = 0;
  int nfaces = 0;
  Vertex **vlist = NULL;
  Face **flist = NULL;
  PlyOtherProp *vert_other, *face_other;
  int has_norm = 0;
  int has_color = 0;

  int i,j,k;
  int elem_count;
  char *elem_name;

  FILE *infile = fopen(filename, "r");
  in_ply = read_ply(infile);

  if (!in_ply) {
    fprintf(stderr, "Can't open %s\n", filename );
  }

  for (i = 0; i < in_ply->num_elem_types; i++) {
    elem_name = setup_element_read_ply(in_ply, i, &elem_count);

    if (equal_strings ("vertex", elem_name)) {
      nverts = elem_count;
      vlist = (Vertex **) malloc (sizeof (Vertex *) * nverts);
      setup_property_ply(in_ply, &vert_props[0]);
      setup_property_ply(in_ply, &vert_props[1]);
      setup_property_ply(in_ply, &vert_props[2]);


      for(j=0; j<in_ply->elems[i]->nprops; j++){
	PlyProperty *prop;
	prop = in_ply->elems[i]->props[j];
	for(k=3; k<9; k++){
	  if(equal_strings(vert_props[k].name, prop->name)){
	    setup_property_ply(in_ply, &vert_props[k]);
	    has_norm = has_norm | (k < 6);
	    has_color = has_color | (k > 5);
	  }
	}
      }

      vert_other = get_other_properties_ply(in_ply, offsetof(Vertex, other_props));

      for (j = 0; j < nverts; j++) {
	vlist[j] = (Vertex *) malloc (sizeof (Vertex));
	get_element_ply(in_ply, (void *) vlist[j]);
      }

    }

    else if (equal_strings ("face", elem_name)) {
      nfaces = elem_count;
      flist = (Face **) malloc (sizeof (Face *) * nfaces);

      setup_property_ply(in_ply, &face_props[0]);
      face_other = get_other_properties_ply(in_ply, offsetof(Face,other_props));
      for (j = 0; j < nfaces; j++) {
	flist[j] = (Face *) malloc (sizeof (Face));
	get_element_ply (in_ply, (void *) flist[j]);
      }
    }
    else
      get_other_element_ply(in_ply);
  }

  //fclose taken care of by close_ply
  close_ply(in_ply);
  free_ply(in_ply);
  numtris = nfaces;
  v0 = new Point3[numtris];
  v1 = new Point3[numtris];
  v2 = new Point3[numtris];
  n0 = new Normal3[numtris];
  n1 = new Normal3[numtris];
  n2 = new Normal3[numtris];
  c0 = new Spectra[numtris];
  c1 = new Spectra[numtris];
  c2 = new Spectra[numtris];

  for (i = 0 ; i < numtris ; i++) {
    v0[i].x = vlist[flist[i]->verts[0]]->x;
    v0[i].y = vlist[flist[i]->verts[0]]->y;
    v0[i].z = vlist[flist[i]->verts[0]]->z;
    v1[i].x = vlist[flist[i]->verts[1]]->x;
    v1[i].y = vlist[flist[i]->verts[1]]->y;
    v1[i].z = vlist[flist[i]->verts[1]]->z;
    v2[i].x = vlist[flist[i]->verts[2]]->x;
    v2[i].y = vlist[flist[i]->verts[2]]->y;
    v2[i].z = vlist[flist[i]->verts[2]]->z;
    v0[i] = xform.apply(v0[i]);
    v1[i] = xform.apply(v1[i]);
    v2[i] = xform.apply(v2[i]);

    if(has_norm){
      n0[i].x = vlist[flist[i]->verts[0]]->nx;
      n0[i].y = vlist[flist[i]->verts[0]]->ny;
      n0[i].z = vlist[flist[i]->verts[0]]->nz;
      n1[i].x = vlist[flist[i]->verts[1]]->nx;
      n1[i].y = vlist[flist[i]->verts[1]]->ny;
      n1[i].z = vlist[flist[i]->verts[1]]->nz;
      n2[i].x = vlist[flist[i]->verts[2]]->nx;
      n2[i].y = vlist[flist[i]->verts[2]]->ny;
      n2[i].z = vlist[flist[i]->verts[2]]->nz;
    }
    else{
      n0[i] = n1[i] = n2[i] = Vector3::Cross(v1[i] - v0[i], v2[i] - v0[i]).Normalize();
    }
    if(has_color){
      c0[i].x = vlist[flist[i]->verts[0]]->diffuse_red/255.0f;
      c0[i].y = vlist[flist[i]->verts[0]]->diffuse_green/255.0f;
      c0[i].z = vlist[flist[i]->verts[0]]->diffuse_blue/255.0f;
      c1[i].x = vlist[flist[i]->verts[1]]->diffuse_red/255.0f;
      c1[i].y = vlist[flist[i]->verts[1]]->diffuse_green/255.0f;
      c1[i].z = vlist[flist[i]->verts[1]]->diffuse_blue/255.0f;
      c2[i].x = vlist[flist[i]->verts[2]]->diffuse_red/255.0f;
      c2[i].y = vlist[flist[i]->verts[2]]->diffuse_green/255.0f;
      c2[i].z = vlist[flist[i]->verts[2]]->diffuse_blue/255.0f;
    }
    else{
      c0[i] = c1[i] = c2[i] = Spectra(.5,.5,.5);
    }
  }

  //free up the structures we malloc'd
  for (j = 0; j < nverts; j++)
    free(vlist[j]);
  free(vlist);
  for (j = 0; j < nfaces; j++)
    free(flist[j]);
  free(flist);
  fprintf(stderr, "Done\n");
}



void WriteVoxFile( char *fname, int *ntris, int modelno,
		   const Tuple3i &grid_dim, const Tuple3f &grid_min,
		   const Tuple3f &grid_max, const Tuple3f &grid_vsize,
		   BitVector* bv, int ****voxels,
		   Point3 **v0, Point3 **v1, Point3 **v2,
		   Normal3 **n0, Normal3 **n1, Normal3 **n2,
		   Spectra **c0, Spectra **c1, Spectra **c2 ){

  int x,y,z,i,j;
  FILE *fp = fopen( fname, "wb" );
  if( !fp ){
    fprintf(stderr, "Couldn't open %s for writing\n", fname);
    exit(-1);
  }

  fprintf(stderr, "\tWriting grid parameters... ");
  fwrite( &grid_dim, sizeof(int), 3, fp );
  fwrite( &grid_min, sizeof(float), 3, fp );
  fwrite( &grid_max, sizeof(float), 3, fp );
  fwrite( &grid_vsize, sizeof(float), 3, fp );
  fprintf(stderr, "Done\n");

  //write bitvector
  fprintf(stderr, "\tWriting bitvector... ");
  bv->WriteToFile(fp);
  fprintf(stderr, "Done\n");

  //write tri_list_offset
  fprintf(stderr, "\tWriting triangle list offsets... ");
  int offset_sum = 0;
  int minusone = -1;
  for( x=0; x<grid_dim.x; x++){
    for( y=0; y<grid_dim.y; y++){
      for( z=0; z<grid_dim.z; z++){
	if( bv->Check(x,y,z) ){
	  fwrite( &offset_sum, sizeof(int), 1, fp );
	  offset_sum += voxels[x][y][z][0] + 1;
	}
	else{
	  fwrite( &minusone, sizeof(int), 1, fp );
	}
      }
    }
  }
  fprintf(stderr, "Done\n");

  //write tri_list
  fprintf(stderr, "\tWriting triangle list... ");
  fwrite( &offset_sum, sizeof(int), 1, fp );
  for( x=0; x<grid_dim.x; x++){
    for( y=0; y<grid_dim.y; y++){
      for( z=0; z<grid_dim.z; z++){
	if( bv->Check(x,y,z) ){
	  for(int i=1; i<=voxels[x][y][z][0]; i++){
	    fwrite( &voxels[x][y][z][i], sizeof(int), 1, fp );
	  }
	  fwrite( &minusone, sizeof(int), 1, fp );
	}
      }
    }
  }
  fprintf(stderr, "Done\n");

  fprintf(stderr, "\tWriting triangle data... ");
  int numtris = 0;
  for( j=0; j<modelno; j++)
    numtris += ntris[j];
  fwrite( &numtris, sizeof(int), 1, fp );
  //write binary triangle data
  //v0,v1,v2,n0,n1,n2,c0,c1,c2
  for( j=0; j<modelno; j++){
    for( i=0; i<ntris[j]; i++){
      fwrite( &v0[j][i], sizeof(float), 3, fp );
      fwrite( &v1[j][i], sizeof(float), 3, fp );
      fwrite( &v2[j][i], sizeof(float), 3, fp );
      fwrite( &n0[j][i], sizeof(float), 3, fp );
      fwrite( &n1[j][i], sizeof(float), 3, fp );
      fwrite( &n2[j][i], sizeof(float), 3, fp );
      fwrite( &c0[j][i], sizeof(float), 3, fp );
      fwrite( &c1[j][i], sizeof(float), 3, fp );
      fwrite( &c2[j][i], sizeof(float), 3, fp );
    }
  }
  fclose(fp);
  fprintf(stderr, "Done\n");

}

void ReadVoxFile( char *fname, bool verbose, Grid& grid,
		  Point3* &v0, Point3* &v1, Point3* &v2,
		  Normal3* &n0, Normal3* &n1, Normal3* &n2,
		  Spectra* &c0, Spectra* &c1, Spectra* &c2 )
{
  FILE *fp = fopen( fname, "rb" );
  if( !fp ){
    fprintf(stderr, "Couldn't open %s for reading\n", fname);
    exit(-1);
  }

  if ( verbose ) fprintf(stderr, "\tReading grid paramters... ");
  fread(&grid.dim, sizeof(int), 3, fp );
  fread(&grid.min, sizeof(float), 3, fp );
  fread(&grid.max, sizeof(float), 3, fp );
  fread(&grid.vsize, sizeof(float), 3, fp );
  if ( verbose ) fprintf(stderr, "Done\n");

  if ( verbose ) fprintf(stderr, "\tReading bitvector... ");
  grid.bitmap = new BitVector( grid.dim, 4 );
  grid.bitmap->ReadFromFile(fp);
  if ( verbose ) fprintf(stderr, "Done\n");

  if ( verbose ) fprintf(stderr, "\tReading triangle list offsets... ");
  grid.trilistOffset = new int[grid.dim.x*grid.dim.y*grid.dim.z];
  int i=0;
  for(int x=0; x<grid.dim.x; x++){
    for(int y=0; y<grid.dim.y; y++){
      for(int z=0; z<grid.dim.z; z++){
	fread( &grid.trilistOffset[i++], sizeof(int), 1, fp );
      }
    }
  }
  if ( verbose ) fprintf(stderr, "Done\n");

  if ( verbose ) fprintf(stderr, "\tReading triangle list... ");
  grid.trilistSize = 0;
  fread( &grid.trilistSize, sizeof(int), 1, fp );
  grid.trilist = new int[grid.trilistSize];
  for( i=0; i<grid.trilistSize; i++)
    fread( &grid.trilist[i], sizeof(int), 1, fp );
  if ( verbose ) fprintf(stderr, "Done\n");

  if ( verbose ) fprintf(stderr, "\tReading triangle data... ");
  fread( &grid.nTris, sizeof(int), 1, fp);
  v0 = new Point3[grid.nTris];
  v1 = new Point3[grid.nTris];
  v2 = new Point3[grid.nTris];
  n0 = new Point3[grid.nTris];
  n1 = new Point3[grid.nTris];
  n2 = new Point3[grid.nTris];
  c0 = new Spectra[grid.nTris];
  c1 = new Spectra[grid.nTris];
  c2 = new Spectra[grid.nTris];
  for( i=0; i<grid.nTris; i++){
    fread( &v0[i], sizeof(float), 3, fp );
    fread( &v1[i], sizeof(float), 3, fp );
    fread( &v2[i], sizeof(float), 3, fp );
    fread( &n0[i], sizeof(float), 3, fp );
    fread( &n1[i], sizeof(float), 3, fp );
    fread( &n2[i], sizeof(float), 3, fp );
    fread( &c0[i], sizeof(float), 3, fp );
    fread( &c1[i], sizeof(float), 3, fp );
    fread( &c2[i], sizeof(float), 3, fp );
  }
  fclose(fp);
  if ( verbose ) fprintf(stderr, "Done\n");
}
