#include <vector>
#include "rapcol.h"

using std::vector;
void LoadPly (const char * file,vector<Tri> &ret);

unsigned int loadModelData(const char * filename,
                           Tri * tri,
                           Neighbor * neigh) {
   std::vector<Tri>triangleList;
   LoadPly(filename,triangleList);
   return 0;
}
