#include <string>
#include <fcntl.h>
#include "Colors.h"
#include "TROOT.h"
#include "ReadoutProcessor.h"
#include "TFile.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

int getdir (std::string dir, std::vector<std::string> &files,std::string nbrRunToProcess)
{
  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(dir.c_str())) == NULL) 
  {
    std::cout<<red << "Error(" << errno << ") opening " << dir << normal<<std::endl;
    return errno;
  }
  while ((dirp = readdir(dp)) != NULL) 
  {
    std::string name=std::string(dirp->d_name);
    std::size_t found = name.rfind("_");
    std::string nbrRun = name.substr(found+1);
    if(nbrRun==nbrRunToProcess+".dat") 
    {
	files.push_back(name);
	std::cout<<green<<"I will process "<<name<<normal<<std::endl;
    }
  }
  closedir(dp);
  return 0;
}

int main(int argc, char *argv[])
{
  gROOT->ProcessLine("#include<vector>");
  if(argc<2)
  {
    std::cout<<"Please provide the file you want to convert"<<std::endl;
    std::exit(2);
  }
  std::string filename=argv[1];
  int numbereventtoprocess=-1;
  if(argc>2) numbereventtoprocess=int32_t(std::stoi(argv[2]));
  if(numbereventtoprocess!=-1)std::cout<<yellow<<"I will process "<<numbereventtoprocess<<" event(s)"<<normal<<std::endl;
  std::size_t found = filename.rfind(".");
  std::string filena=filename;
  filena=filena.erase(found);
  TFile file((filena+".root").c_str(),"RECREATE",filena.c_str(),2);
  found = filena.rfind("_");
  std::string nbrRun = filena.substr(found+1);  
  found = filena.rfind("/");
  std::string path ="./";  
  if(found!=std::string::npos)path=filena.substr(0,found+1);
  std::vector<std::string> FilesTopProcess;
  getdir(path,FilesTopProcess,nbrRun);
  if (file.IsOpen() != true) 
  {
    std::cout << red << "Impossible to open " << filena<< normal << std::endl;
    std::exit(1);
  }
  ReadoutProcessor Pr(numbereventtoprocess,&file);
  Pr.init();
  int retour=0;
  for(unsigned int i=0;i!=FilesTopProcess.size();++i)
  {
    int32_t _fdIn= ::open((path+FilesTopProcess[i]).c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
    if (_fdIn<0)
    {
      perror("Can't open file :");
      Pr.finish();
      file.Close();
      return 1;
    }
    else std::cout<<green<<"Opening "<<FilesTopProcess[i]<<normal<<std::endl;
    retour=Pr.readstream(_fdIn);
    if(retour==15)
    {
      Pr.finish();
      file.Close();
      return 0;
    }
  }
  Pr.finish();
  file.Close();
  return retour;
}
