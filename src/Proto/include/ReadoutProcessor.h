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
class ReadoutProcessor
{
public:
  ReadoutProcessor(int NbrEventToProcess):numbereventtoprocess(NbrEventToProcess){}
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
  RAWData data;
  TTree* dataTree=nullptr;
  TTree* noiseTree=nullptr; 
  TBranch *bEventNumber = nullptr;
  TBranch *bNumberOfHits = nullptr;
  TBranch *bTDCChannel =nullptr;
  TBranch *bTDCTimeStamp = nullptr;
  TBranch *bTDCTimeStampReal = nullptr;
  TBranch *bWitchSide = nullptr;
  TBranch *bMezzanine = nullptr;
  TBranch *bEventNumber2 = nullptr;
  TBranch *bNumberOfHits2 = nullptr;
  TBranch *bTDCChannel2 =nullptr;
  TBranch *bTDCTimeStampReal2 = nullptr;
  TBranch *bWitchSide2 = nullptr;
  TBranch *bMezzanine2 = nullptr;
  ChamberCounters _counters;
  int numbereventtoprocess;
};
#endif 
