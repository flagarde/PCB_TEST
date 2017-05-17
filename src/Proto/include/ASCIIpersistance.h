#ifndef ASCIIpersistance_HH
#define ASCIIpersistance_HH

#include <fstream>
#include <iostream>
#include <string>

class ASCIIpersistance
{
 public:
  virtual bool ASCIIwrite(std::ostream& oflux=std::cout) const=0;
  virtual bool ASCIIread(std::istream& iflux=std::cin)=0;
  
  bool ASCIIfilewrite(const char* filename) const { std::ofstream f(filename); return (f.good() &&  ASCIIwrite(f));}
  bool ASCIIfileread(const char* filename)        { std::ifstream f(filename); return (f.good() &&  ASCIIread(f));}

  bool ASCIIfilewrite(std::string filename) const { return ASCIIfilewrite(filename.c_str());}
  bool ASCIIfileread(std::string filename)        { return ASCIIfileread (filename.c_str());}

  bool ASCIIfileappend(const char* filename) const { std::ofstream f(filename,std::ios::app); return (f.good() &&  ASCIIwrite(f));}
  bool ASCIIfileappend(std::string filename) const { return ASCIIfileappend(filename.c_str());}
};

#endif
