#ifndef Plots_h
#define Plots_h
#include "TCanvas.h"
#include "TFile.h"
#include <vector>
#include "TObject.h"
class TCanvasDivided
{
  public :
  TCanvasDivided():m_x(1),m_y(1){}
  TCanvasDivided(int n):m_x(n),m_y(n){}
  TCanvasDivided(int x,int y):m_x(x),m_y(y){}
  void setName(std::string& name)
  {
    m_name=name;
  }
  void setName(const char* name)
  {
    m_name=std::string(name);
  }
  void add(TObject* plot)
  {
    if(rolling%(m_x*m_y+1)==0||nbr==0) 
    {
      rolling=1;
      nbr++;
      cans.push_back(new TCanvas(parseName(nbr).c_str(),parseName(nbr).c_str()));
      cans[cans.size()-1]->Divide(m_x,m_y);
    }
    std::cout<<rolling<<std::endl;
    cans[cans.size()-1]->cd(rolling%(m_x*m_y+1));
    ++rolling;
    plot->Draw();
  }
  void write(TFile * file)
  {
    file->cd();
    for(unsigned int i=0;i!=cans.size();++i)
    {
      cans[i]->Write();    
      delete cans[i];
    }
  }
  private:
  std::string parseName(int i)
  {
    std::size_t found=m_name.rfind("*");
    std::string nbrCanvas=std::to_string(i);
    std::string dump=m_name;
    if(found==std::string::npos)
    {
      return m_name+"_"+nbrCanvas;
    }
    else return dump.replace(found,1,nbrCanvas);
  }
  std::string m_name{""};
  std::vector<TCanvas*> cans;
  int rolling{1};
  int nbr{0};
  int m_x{1};
  int m_y{1};
};
#endif 
