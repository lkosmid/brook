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
#define PB(X,Y,Z) ret.push_back(string3(X,Y,Z));
struct string3 {
  string s[3];
  string3(string a, string b, string c) {
    s[0]=a;s[1]=b;s[2]=c;
  }
};
vector<string3> VectorTypes() {
  vector<string3> ret;
  PB("vec<float,1> ","float","1");
  PB("vec<int,1> ","int","1");
  PB("vec<char,1> ","char","1");
  PB("vec<vec<float,1>,1> ","vec<float,1>","1")
  PB("vec<vec<float,1>,2> ","vec<float,1>","2")
    PB("vec<vec<float,1>,3> ","vec<float,1>","3")
   PB("vec<vec<float,1>,4> ","vec<float,1>","4")

    PB("vec<vec<int,1>,1> ","vec<int,1>","1")
    PB("vec<vec<int,1>,2> ","vec<int,1>","2")
    PB("vec<vec<int,1>,3> ","vec<int,1>","3")
    PB("vec<vec<int,1>,4> ","vec<int,1>","4")

    PB("vec<vec<char,1>,1> ","vec<char,1>","1")
    PB("vec<vec<char,1>,2> ","vec<char,1>","2")
    PB("vec<vec<char,1>,3> ","vec<char,1>","3")
    PB("vec<vec<char,1>,4> ","vec<char,1>","4")
    
  return ret;
}
vector<string3> BasicTypes() {
  vector<string3> ret;
  PB("int","int","1");
  PB("char","char","1");
  PB("float","float","1");
  PB("double","double","1");
  PB("unsigned int","int","1");

  return ret;
}
vector<string3> OperatorTypes() {
  vector<string3> ret;
  PB("vec<VALUE,1> ","VALUE","1");
  PB("vec<VALUE,2> ","VALUE","2");
  PB("vec<VALUE,3> ","VALUE","3");
  PB("vec<VALUE,4> ","VALUE","4");
  return ret;
}

vector<string3> GeneralTypes() {
  vector <string3> ret = BasicTypes();
  vector<string3> vt =VectorTypes();
  ret.insert(ret.end(),vt.begin(),vt.end());
  return ret;
}

#undef PB
vector<string3> operTypes = OperatorTypes();
vector<string3> basicTypes = BasicTypes();
vector <string3> generalTypes = GeneralTypes();
vector<string3> vectorTypes = VectorTypes();
string preprocessTemplates (string s, vector<string3> replacementList) {
  string ret;
  s=findReplace (s,"template <class BRT_TYPE>","");
  //s=findReplace(s,"typename","");
  s=findReplace(s,"GCCTYPENAME","typename");
  s=findReplace(s,"INTERNALTYPENAME","");
  s=findReplace(s,"MSC_VER","ARRGH");
  for (unsigned int i=0;i<replacementList.size();++i) {
    
    string tmp = findReplace(s,"BRT_TYPE::TYPE",replacementList[i].s[1]);
    tmp = findReplace(tmp,"BRT_TYPE::size",replacementList[i].s[2]);
    ret+=tmp =findReplace(tmp,"BRT_TYPE",replacementList[i].s[0]);
    
  }
  return ret;
}
string removeTypenames (string in) {
  for (unsigned int i=0;i<basicTypes.size();++i) {
    string findme ("typename "+basicTypes[i].s[0]);
    printf( "find %s\n",findme.c_str());
    in = findReplace(in,findme,basicTypes[i].s[0]);
  }
  in = findReplace (in, "typename vec","vec");
  in = findReplace (in, "typename VALUE","VALUE");
  return in;
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
  string pre,general,firstpost,vectoronly,post,operonly,lastpost;
  {
    general=findBetween(in,"GENERAL_TEMPLATIZED_FUNCTIONS",pre,post);
    vectoronly=findBetween(post,"VECTOR_TEMPLATIZED_FUNCTIONS",firstpost,post);
    operonly=findBetween(post,"OPERATOR_TEMPLATIZED_FUNCTIONS",post,lastpost);
  }
  o << findReplace(pre,"BRTVECTOR_HPP","VC6VECTOR_HPP") <<std::endl;
  o << removeTypenames (preprocessTemplates(general,generalTypes))<<std::endl;
  o << firstpost<<std::endl;
  o << removeTypenames(preprocessTemplates(vectoronly,vectorTypes))<<std::endl;
  o << post <<std::endl;
  o << removeTypenames(preprocessTemplates(operonly,operTypes))<<std::endl;
  o << lastpost;
  o << std::endl << "#undef __MY_VC6_HEADER" <<std::endl;
  o.close();

}
