#include "ReadoutProcessor.h" 
#include <set>
#include <algorithm>
#include "Global.h"
#include "Predicate.h"
void ReadoutProcessor::init()
{
  _maxBCID_histo= new TH1F("MAX_BCID","Maximum BCID",200,0,200);
  _maxBCID_histo->GetXaxis()->SetCanExtend(true);
  _maxBCID_histozoom= new TH1F("MAX_BCID_zoom","Maximum BCID",200,0,200);
  _triggerPerReadout=new TH1F("triggerPerReadout","number of triggers per readout",50,0,50);
}

void ReadoutProcessor::finish()
{
  _maxBCID_histo->Write();
  _maxBCID_histozoom->Write();
  _triggerPerReadout->Write();
  std::string labels[3]={"ALL", "CHAMBER", "MEZZANINE"};
  _counters.write(labels);
}

void ReadoutProcessor::processReadout(TdcChannelBuffer &tdcBuf)
{
  _maxBCID=0;
  std::set<uint16_t> BCIDwithTrigger;
  for (TdcChannel* it=tdcBuf.begin(); it != tdcBuf.end(); ++it)
    {
      uint16_t bcid=it->bcid();
      if (_maxBCID<bcid) _maxBCID=bcid;
      if (it->channel()==triggerChannel) BCIDwithTrigger.insert(bcid); 
    }
  _maxBCID_histo->Fill(_maxBCID);
  _maxBCID_histozoom->Fill(_maxBCID);
  _triggerPerReadout->Fill(BCIDwithTrigger.size());
  //eventually here put a filter on the  BCIDwithTrigger set (like remove first ones, last ones, close ones)

  
  TdcChannel* eventStart=tdcBuf.begin();
  TdcChannel* eventEnd=NULL;
  //std::cout << "Nombre de triggers = " << BCIDwithTrigger.size() << std::endl;
  for (std::set<uint16_t>::iterator it=BCIDwithTrigger.begin(); it !=BCIDwithTrigger.end(); ++it)
    {
      eventEnd=std::partition(eventStart,tdcBuf.end(),TdcChannelBcidpredicate(*it,-6,-3));
      processTrigger(eventStart,eventEnd);
      eventStart=eventEnd;
    }
  processNoise(eventStart,tdcBuf.end());
}


void ReadoutProcessor::processTrigger(TdcChannel* begin,TdcChannel* end)
{
  TdcChannel* mezzStart=begin;
  TdcChannel* mezzEnd=NULL;
  for (int chamber=1; chamber <=2; ++chamber)
    for (int mezz=0; mezz <2; ++mezz)
      {
	mezzEnd=std::partition(mezzStart,end,TdcChannelMezzaninePredicate(chamber,mezz));
	processMezzanine(mezzStart,mezzEnd);
	mezzStart=mezzEnd;
      }
  if (NULL != mezzEnd && end != mezzEnd) std::cout << "WARNING WARNING mess here" << std::endl;
  _counters.newSet();
}

void ReadoutProcessor::processNoise(TdcChannel* begin,TdcChannel* end)
{
}

bool isTrigger(TdcChannel& c) {return c.channel()==triggerChannel;}

void ReadoutProcessor::processMezzanine(TdcChannel* begin,TdcChannel* end)
{
  int trigCount=std::count_if(begin,end,isTrigger);
  if (trigCount != 1) return;
  //std::cout << " trigCount pour nhit = " << int(end-begin) << std::endl;
      TdcChannel *trigger=std::find_if(begin,end,isTrigger);
  unsigned int valeur[2]={(unsigned int)trigger->chamber(),(unsigned int)trigger->mezzanine()};
  if (int(end-begin)>1) //at least one hit more than the trigger
    {
      _counters.add(1,valeur);
    }
  else
    _counters.add(0,valeur);
}
