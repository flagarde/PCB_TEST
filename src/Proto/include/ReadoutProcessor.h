#ifndef ReadoutProcessor_h
#define ReadoutProcessor_h 
#include "TdcChannelBuffer.h"
#include "TH1F.h"
#include "TH2F.h"
#include "GG_counter.h"
#include <cstdint>
#include "TBranch.h"
#include "TTree.h"
#include "RAWData.h"
#include "TProfile.h"
#include "TFile.h"
class ReadoutProcessor
{
public:
  ReadoutProcessor(int NbrEventToProcess,TFile* fol):numbereventtoprocess(NbrEventToProcess),folder(fol){}
  void init();
  int readstream(int32_t _fdIn);
  void processReadout(TdcChannelBuffer &tdcBuf);
  void processTrigger(TdcChannel* begin,TdcChannel* end);
  void processMezzanine(TdcChannel* begin,TdcChannel* end);
  void processNoise(TdcChannel* begin,TdcChannel* end);
  void finish();
private:
  uint16_t _maxBCID;
  TH1F* _maxBCID_histo=nullptr;
  TH1F* _maxBCID_histozoom=nullptr;
  TH1F* _triggerPerReadout=nullptr;
  TH2F* _triggerPerReadoutPerMezzanine=nullptr;
  TProfile* _noisehitspersecond=nullptr;
  RAWData _data;
  TTree* _dataTree=nullptr;
  TTree* _noiseTree=nullptr; 
  TBranch *_bEventNumber = nullptr;
  TBranch *_bNumberOfHits = nullptr;
  TBranch *_bTDCChannel =nullptr;
  TBranch *_bTDCTimeStamp = nullptr;
  TBranch *_bTDCTimeStampReal = nullptr;
  TBranch *_bWitchSide = nullptr;
  TBranch *_bMezzanine = nullptr;
  TBranch *_bEventNumber2 = nullptr;
  TBranch *_bNumberOfHits2 = nullptr;
  TBranch *_bTDCChannel2 =nullptr;
  TBranch *_bTDCTimeStampReal2 = nullptr;
  TBranch *_bWitchSide2 = nullptr;
  TBranch *_bMezzanine2 = nullptr;
  ChamberCounters _counters;
  int32_t numbereventtoprocess;
  int32_t totalevent{0};
  TFile* folder=nullptr;
  std::map<int,TH1F*> _Multiplicity;
  std::map<int,TH2F*> _T1mT2;
  std::map<int,TH1F*> _T1mT2Ch;
  std::map<int,TH1F*> _T1mT2Chamber;
  std::map<int,TH2F*> _Position;
  std::map<int,TH2F*> _Longueur;
  std::map<int,TH1F*> _NbrCluster;
  std::map<int,TH1F*> _MultiCluster;
};
#endif 
