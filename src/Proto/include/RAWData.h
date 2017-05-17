#ifndef RAWDATA_h
#define RAWDATA_h
#include<vector>
#include<array>
#include "Colors.h"
class RAWData
{
  public:
    RAWData()
    {
      TDCCh = new std::vector<int>;   // List of hits and their channels
      TDCTS = new std::vector<double>; // List of the corresponding timestamps
      TDCTSReal = new std::vector<double>;
      WitchSide = new std::vector<int>;
      Mezzanine = new std::vector<int>;
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      WitchSide->clear();
      Mezzanine->clear();
      iEvent=0;
      TDCNHits=0;
      iNoise=0;
    }
    ~RAWData()
    {
      delete TDCCh;
      delete TDCTS;
      delete TDCTSReal;
      delete WitchSide;
      delete Mezzanine;
    }
    void Reserve(int i)
    {
      TDCCh->reserve(i);
      TDCTS->reserve(i);
      TDCTSReal->reserve(i);
      WitchSide->reserve(i);
      Mezzanine->reserve(i);
    }
    void Push_back(int side,int channel,int mezzanine,double timestamp,double timestamptrigger)
    {
      //std::cout<<red<<timestamp<<"  "<<timestamptrigger<<normal<<std::endl;
      TDCCh->push_back(channel);
      TDCTSReal->push_back(timestamp);
      TDCTS->push_back(float(timestamp-timestamptrigger));
      std::cout<<std::setprecision (std::numeric_limits<double>::digits10+1)<<blue<<timestamp-timestamptrigger<<normal<<std::endl;
      WitchSide->push_back(side);
      Mezzanine->push_back(mezzanine);
      TDCNHits++;
    }
    void Push_back(int side,int channel,int mezzanine,double timestamp)
    {
      TDCCh->push_back(channel);
      TDCTSReal->push_back(timestamp);
      WitchSide->push_back(side);
      Mezzanine->push_back(mezzanine);
      TDCNHits++;
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      WitchSide->clear();
      Mezzanine->clear();
      TDCNHits=0;
    }
    void OneEvent()
    {
      iEvent++;
    }
    void OneNoise()
    {
      iNoise++;
    }
    int iEvent;     //Event i
    int iNoise;
    int TDCNHits;   //Number of hits in event i
    std::vector<int>* TDCCh;      //List of channels giving hits per event
    std::vector<double>* TDCTS;      //List of the corresponding time stamps
    std::vector<double>* TDCTSReal;
    std::vector<int>* WitchSide;
    std::vector<int>* Mezzanine;
};
#endif
