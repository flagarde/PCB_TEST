#ifndef RAWDATA_h
#define RAWDATA_h
#include<vector>
#include<array>
std::array<std::array<int,32>,2>TDCchannelToStrip
{{
  {15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0},
  {16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31}
}};
class RAWDataTriggered
{
  public:
    RAWDataTriggered()
    {
      TDCCh = new std::vector<int>;   // List of hits and their channels
      TDCTS = new std::vector<float>; // List of the corresponding timestamps
      TDCTSReal = new std::vector<float>;
      WitchSide = new std::vector<int>;
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      WitchSide->clear();
      iEvent=0;
      TDCNHits=0;
    }
    ~RAWDataTriggered()
    {
      delete TDCCh;
      delete TDCTS;
      delete TDCTSReal;
      delete WitchSide;
    }
    void Reserve(int i)
    {
      TDCCh->reserve(i);
      TDCTS->reserve(i);
      TDCTSReal->reserve(i);
      WitchSide->reserve(i);
      TDCNHits=i;
    }
    void Push_back(int mezzanine,int channel,float timestamp,float timestamptrigger)
    {
      std::cout<<mezzanine<<"  "<<channel<<"  "<<TDCchannelToStrip[mezzanine-1][channel]<<std::endl;
      TDCCh->push_back(TDCchannelToStrip[mezzanine-1][channel]);
      TDCTS->push_back(timestamp-timestamptrigger);
      TDCTSReal->push_back(timestamp);
      WitchSide->push_back(channel%2);
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      WitchSide->clear();
    }
    void OneEvent()
    {
      iEvent++;
    }
    int iEvent;     //Event i
    int TDCNHits;   //Number of hits in event i
    std::vector<int>* TDCCh;      //List of channels giving hits per event
    std::vector<float>* TDCTS;      //List of the corresponding time stamps
    std::vector<float>* TDCTSReal;
    std::vector<int>* WitchSide;
};

class RAWDataNotTriggered
{
  public:
    RAWDataNotTriggered()
    {
      TDCCh = new std::vector<int>;   // List of hits and their channels
      TDCTS = new std::vector<float>; // List of the corresponding timestamps
      WitchSide = new std::vector<int>;
      TDCCh->clear();
      TDCTS->clear();
      WitchSide->clear();
      iEvent=0;
      TDCNHits=0;
    }
    ~RAWDataNotTriggered()
    {
      delete TDCCh;
      delete TDCTS;
      delete WitchSide;
    }
    void Reserve(int i)
    {
      TDCCh->reserve(i);
      TDCTS->reserve(i);
      WitchSide->reserve(i);
      TDCNHits=i;
    }
    void Push_back(int mezzanine,int channel,float timestamp)
    {
      std::cout<<mezzanine<<"  "<<channel<<"  "<<TDCchannelToStrip[mezzanine-1][channel]<<std::endl;
      TDCCh->push_back(TDCchannelToStrip[mezzanine-1][channel]);
      TDCTS->push_back(timestamp);
      WitchSide->push_back(channel%2);
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      WitchSide->clear();
    }
    void OneEvent()
    {
      iEvent++;
    }
    int iEvent;     //Event i
    int TDCNHits;   //Number of hits in event i
    std::vector<int>* TDCCh;      //List of channels giving hits per event
    std::vector<float>* TDCTS;      //List of the corresponding time stamps
    std::vector<int>* WitchSide;
};
#endif
