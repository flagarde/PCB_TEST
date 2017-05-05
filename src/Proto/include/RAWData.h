#ifndef RAWDATA_h
#define RAWDATA_h
#include<vector>
class RAWDataTriggered
{
  public:
    RAWDataTriggered()
    {
      TDCCh = new std::vector<int>;   // List of hits and their channels
      TDCTS = new std::vector<float>; // List of the corresponding timestamps
      TDCTSReal = new std::vector<float>;
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
      iEvent=0;
      TDCNHits=0;
    }
    ~RAWDataTriggered()
    {
      delete TDCCh;
      delete TDCTS;
      delete TDCTSReal;
    }
    void Reserve(int i)
    {
      TDCCh->reserve(i);
      TDCTS->reserve(i);
      TDCTSReal->reserve(i);
      TDCNHits=i;
    }
    void Push_back(int channel,float timestamp,float timestamptrigger)
    {
      TDCCh->push_back(channel);
      TDCTS->push_back(timestamp-timestamptrigger);
      TDCTSReal->push_back(timestamp);
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
      TDCTSReal->clear();
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
};

class RAWDataNotTriggered
{
  public:
    RAWDataNotTriggered()
    {
      TDCCh = new std::vector<int>;   // List of hits and their channels
      TDCTS = new std::vector<float>; // List of the corresponding timestamps
      TDCCh->clear();
      TDCTS->clear();
      iEvent=0;
      TDCNHits=0;
    }
    void Reserve(int i)
    {
      TDCCh->reserve(i);
      TDCTS->reserve(i);
      TDCNHits=i;
    }
    void Push_back(int channel,float timestamp)
    {
      TDCCh->push_back(channel);
      TDCTS->push_back(timestamp);
    }
    void Reset()
    {
      TDCCh->clear();
      TDCTS->clear();
    }
    ~RAWDataNotTriggered()
    {
      delete TDCCh;
      delete TDCTS;
    }
    void OneEvent()
    {
      iEvent++;
    }
    int iEvent;     //Event i
    int TDCNHits;   //Number of hits in event i
    std::vector<int>* TDCCh;      //List of channels giving hits per event
    std::vector<float>* TDCTS;      //List of the corresponding time stamps
};
#endif
