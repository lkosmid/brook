#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

string findReplace (string s, string find, string replace) {
  unsigned int where;
  while ((where=s.find(find))!=string::npos) {
    s = s.substr(0,where)+replace+s.substr(where+find.length());
  }
  return s;
}
#define PB(X) ret.push_back(X);
vector<string> VectorTypes() {
  vector<string> ret;
  PB("vec<float,1> ");
  PB("vec<int,1> ");
  PB("vec<char,1> ");
  PB("vec<vec<float,1>,1> ")
  PB("vec<vec<float,1>,2> ")
  PB("vec<vec<float,1>,3> ")
  PB("vec<vec<float,1>,4> ")

  PB("vec<vec<int,1>,1> ")
  PB("vec<vec<int,1>,2> ")
  PB("vec<vec<int,1>,3> ")
  PB("vec<vec<int,1>,4> ")

  PB("vec<vec<char,1>,1> ")
  PB("vec<vec<char,1>,2> ")
  PB("vec<vec<char,1>,3> ")
  PB("vec<vec<char,1>,4> ")
    
  return ret;
}
vector<string> GeneralTypes() {
  vector<string> ret;
  PB("int");
  PB("char");
  PB("unsigned int");
  PB("float");
  PB("double");
  vector<string> vt =VectorTypes();
  ret.insert(ret.end(),vt.begin(),vt.end());
  return ret;
}

#undef PB
vector <string> generalTypes = GeneralTypes();
vector<string> vectorTypes = VectorTypes();
string preprocessTemplates (string s, vector<string> replacementList) {
  string ret;
  s=findReplace (s,"template <class BRT_TYPE>","");
  for (unsigned int i=0;i<replacementList.size();++i) {
    ret+=findReplace(s,"BRT_TYPE",replacementList[i]);
  }
  return ret;
}

string findBetween (string in, string name, string &pre, string &post) {
  pre=in;
  unsigned int premarker = pre.find ("#define "+name);
  string s;
  if (premarker!=string::npos) {
    s= pre.substr(premarker);
    pre = pre.substr(0,premarker);
    string postmarkstr("#undef "+name);
    unsigned int postmarker = s.find(postmarkstr);
    if( postmarker!=string::npos) {
      postmarker+=postmarkstr.length();
      post = s.substr(postmarker);
      s = s.substr(0,postmarker)+"\n";
      printf ("found %s\n",name.c_str());
    }
  } 
  return s;
}
int main (int argc, char ** argv) {
  FILE * fp = fopen (argv[1],"r");
  struct stat st;
  stat (argv[1],&st);
  char * mem = (char *)malloc(st.st_size+1);
  fread(mem,st.st_size,1,fp);
  fclose(fp);
  string in(mem,st.st_size);
  free (mem);
  ofstream o(argv[2]);
  string general,firstpre,lastpre,firstpost,vectoronly,lastpost;
  {
    string post,pre;
    general=findBetween(in,"GENERAL_TEMPLATIZED_FUNCTIONS",pre,post);
    vectoronly=findBetween(pre,"VECTOR_TEMPLATIZED_FUNCTIONS",firstpre,lastpre);
    vectoronly+="\n"+findBetween(post,"VECTOR_TEMPLATIZED_FUNCTIONS",firstpost,lastpost);
  }
  o << firstpre<<"\n"<<lastpre;
  o << preprocessTemplates(general,generalTypes)<<"\n";
  o << firstpost<<"\n";
  o << preprocessTemplates(vectoronly,vectorTypes)<<"\n";
  o << lastpost;
  o.close();

}
