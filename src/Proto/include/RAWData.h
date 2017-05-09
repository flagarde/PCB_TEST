#ifndef RAWDATA_h
#define RAWDATA_h
#include<vector>
#include<array>

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
    }
    void Push_back(int channel,float timestamp,float timestamptrigger)
    {
      TDCTS->push_back(timestamp-timestamptrigger);
      TDCCh->push_back(channel);
      TDCTSReal->push_back(timestamp);
      WitchSide->push_back(channel%2);
      TDCNHits++;
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      WitchSide->clear();
      TDCNHits=0;
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
    }
    void Push_back(int channel,float timestamp)
    {
      TDCCh->push_back(channel);
      TDCTS->push_back(timestamp);
      WitchSide->push_back(channel%2);
      TDCNHits++;
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      WitchSide->clear();
      TDCNHits=0;
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
