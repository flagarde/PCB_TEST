#include <string>
#include <fcntl.h>
#include "Colors.h"
#include "TROOT.h"
#include "readstream.h"
#include "TFile.h"
int main(int argc, char *argv[])
{
  gROOT->ProcessLine("#include<vector>");
  if(argc<2)
  {
    std::cout<<"Please provide the file you want to convert"<<std::endl;
    std::exit(2);
  }
  std::string filename=argv[1];
  if(argc>2) numbereventtoprocess=std::stoi(argv[2]);
  std::size_t found = filename.rfind(".");
  std::string filena=filename;
  filena=filena.erase(found);
  TFile file((filena+".root").c_str(),"RECREATE",filena.c_str(),2);
  if (file.IsOpen() != true) 
  {
    std::cout << red << "Impossible to open " << filena<< normal << std::endl;
    std::exit(1);
  }
  int32_t _fdIn= ::open(filename.c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
  if (_fdIn<0)
  {
    perror("Can't open file :");
    return 1;
  }
  ReadoutProcessor Pr;
  Pr.init();
  int retour=readstream(_fdIn,Pr);
  Pr.finish();
  file.Close();
  return retour;
}
