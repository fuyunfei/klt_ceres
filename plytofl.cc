
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "pnmio.h"
#include "klt.h"

using namespace std;
extern "C" KLT_FeatureList plytofl(KLT_FeatureList);
KLT_FeatureList plytofl ( KLT_FeatureList fl) {
  string line;
  ifstream myfile;
  ofstream output;
  myfile.open ("1.ply");
 for (int i=0;i<10;i++){
 getline (myfile,line);
 cout << line << '\n';
 }
 int i=0;
 for( std::string line;std::getline(myfile, line);i++ )
 {
     std::istringstream in(line);      //make a stream for the line itself
      double x, y, z;
      int r,g,b;

      in >> x >> y >> z>>r>>g>>b;       //now read the whitespace-separated floats

      fl->feature[i]->x= int(1920/2+x*(double)1781/z);
      fl->feature[i]->y= int(1080/2+y*(double)1781/z);
       
}
     myfile.close();
     
     return fl;
}
