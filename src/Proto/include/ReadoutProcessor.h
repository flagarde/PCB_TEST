#ifndef ReadoutProcessor_h
#define ReadoutProcessor_h 
#include "TdcChannelBuffer.h"
#include "TH1F.h"
#include "TH2F.h"
#include "GG_counter.h"
class ReadoutProcessor
{
public:
  ReadoutProcessor() {}
  void init();
  void processReadout(TdcChannelBuffer &tdcBuf);
  void processTrigger(TdcChannel* begin,TdcChannel* end);
  void processMezzanine(TdcChannel* begin,TdcChannel* end);
  void processNoise(TdcChannel* begin,TdcChannel* end);
  void finish();
private:
  uint16_t _maxBCID;
  TH1F* _maxBCID_histo=NULL;
  TH1F* _maxBCID_histozoom=NULL;
  TH1F* _triggerPerReadout=NULL;
  ChamberCounters _counters;
};
#endif 
