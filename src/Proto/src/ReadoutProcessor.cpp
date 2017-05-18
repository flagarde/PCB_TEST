#include "ReadoutProcessor.h" 
#include <set>
#include <algorithm>
#include "Global.h"
#include "Predicate.h"
#include "buffer.h"


//#define LAURENT_STYLE

int ReadoutProcessor::readstream(int32_t _fdIn)
{
  if (_fdIn<=0) return 0;
  uint32_t _event=0;
  uint32_t maxsize=0x100000;
  levbdim::buffer b(maxsize);
  TdcChannelBuffer tdcBuf(maxsize);
  while (true)
  {
    tdcBuf.clear();
    uint32_t theNumberOfDIF=0;
    int ier=::read(_fdIn,&_event,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore %d \n ",ier); return 0;
	  }
    else printf("Event read %d \n",_event);
    if(numbereventtoprocess==_event)return 0;
    ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore number of DIF %d \n ",ier); return 0;
	  }
    //else printf("Number of DIF found %d \n",theNumberOfDIF);
    for (uint32_t idif=0;idif<theNumberOfDIF;idif++) 
	  {
	    uint32_t bsize=0;
	    ier=::read(_fdIn,&bsize,sizeof(uint32_t));
	    if (ier<=0)
	    {
	      printf("Cannot read anymore  DIF Size %d \n ",ier);return 0;
	    }
	    ier=::read(_fdIn,b.ptr(),bsize);
	    if (ier<=0)
	    {
	      printf("Cannot read anymore Read data %d \n ",ier);return 0;
	    }
	    b.setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
	    b.uncompress();
	    if (b.detectorId() != 110) continue;
	    uint32_t* ibuf=(uint32_t*) b.payload();
	    //for (int i=0;i<7;i++)  printf("%d ",ibuf[i]);
	    //uint64_t absbcid=ibuf[3]; absbcid=(absbcid<<32)|ibuf[2];
	    //printf("\n event number %d, GTC %d, ABSBCID %lu, mezzanine number %u, ",ibuf[0],ibuf[1]&0xFFFF,absbcid,ibuf[4]);
	    //printf("IP address %u.%u.%u.%u,",ibuf[5]&0xFF,(ibuf[5]>>8)&0xFF,(ibuf[5]>>16)&0xFF,(ibuf[5]>>24)&0xFF);
	    //uint32_t nch=ibuf[6];
	    //printf("\n channels -> %d \n",nch);
	    if (ibuf[6]>0)
	    {
	      uint8_t* cbuf=( uint8_t*)&ibuf[7];
	      for (int i=0;i<ibuf[6];i++)
		    {
		      tdcBuf.addChannel(&cbuf[8*i]);
		      TdcChannel &c=tdcBuf.last();
		      c.setstrip(ibuf[4],(ibuf[5]>>24)&0xFF);
		    }
	    }
	  } // end loop on DIF
    processReadout(tdcBuf);
  }
} 


void ReadoutProcessor::init()
{
  _maxBCID_histo= new TH1F("MAX_BCID","Maximum BCID",200,0,200);
  _maxBCID_histo->GetXaxis()->SetCanExtend(true);
  _maxBCID_histozoom= new TH1F("MAX_BCID_zoom","Maximum BCID",200,0,200);
  _triggerPerReadout=new TH1F("triggerPerReadout","number of triggers per readout",50,0,50);
  _triggerPerReadoutPerMezzanine=new TH2F("triggerPerReadoutPerMezzanine","number of triggers per readout per mezzanine",50,0,50,4,12,16);
  _triggerPerReadoutPerMezzanine->GetXaxis()->SetTitle("Number of trigger in readout");
  _triggerPerReadoutPerMezzanine->GetYaxis()->SetTitle("Mezzanine");
  data.Reserve(1000);
  dataTree=new TTree("RAWData","RAWData"); 
  noiseTree=new TTree("RAWNoise","RAWNoise"); 
  bEventNumber = dataTree->Branch("EventNumber",  &data.iEvent,50000,0);
  bNumberOfHits = dataTree->Branch("number_of_hits", &data.TDCNHits,50000,0);
  bTDCChannel = dataTree->Branch("TDC_channel",  &data.TDCCh,50000,0);
  bTDCTimeStamp = dataTree->Branch("TDC_TimeStamp", &data.TDCTS,50000,0);
  bTDCTimeStampReal = dataTree->Branch("TDC_TimeStampReal", &data.TDCTSReal,50000,0);
  bWitchSide = dataTree->Branch("WichSide",  &data.WitchSide,50000,0);
  bMezzanine = dataTree->Branch("Mezzanine",  &data.Mezzanine,50000,0);
  bEventNumber2 = noiseTree->Branch("EventNumber",  &data.iNoise,50000,0);
  bNumberOfHits2 = noiseTree->Branch("number_of_hits", &data.TDCNHits,50000,0);
  bTDCChannel2 = noiseTree->Branch("TDC_channel",  &data.TDCCh,50000,0);
  bTDCTimeStampReal2 = noiseTree->Branch("TDC_TimeStampReal",&data.TDCTSReal,50000,0);
  bWitchSide2 = noiseTree->Branch("WichSide",  &data.WitchSide,50000,0);
  bMezzanine2 = noiseTree->Branch("Mezzanine",  &data.Mezzanine,50000,0);
}

void ReadoutProcessor::finish()
{
  _maxBCID_histo->Write();
  _maxBCID_histozoom->Write();
  _triggerPerReadout->Write();
  _triggerPerReadoutPerMezzanine->Write();
  std::string labels[3]={"ALL", "CHAMBER", "MEZZANINE"};
  _counters.write(labels);
  dataTree->Write();
  noiseTree->Write();
}

void ReadoutProcessor::processReadout(TdcChannelBuffer &tdcBuf)
{
  _maxBCID=0;
  std::set<std::pair<uint16_t,double>> BCIDwithTrigger;
  std::map<int,std::vector<uint16_t> > BCIDwithTriggerPerMezzanine;
  for (TdcChannel* it=tdcBuf.begin(); it != tdcBuf.end(); ++it)
  {
    uint16_t bcid=it->bcid();
    if (_maxBCID<bcid) _maxBCID=bcid;
    if (it->channel()==triggerChannel) 
    { 
      BCIDwithTrigger.insert(std::pair<uint16_t,double>(bcid,it->tdcTime()));  
      BCIDwithTriggerPerMezzanine[it->mezzanine()].push_back(bcid);
    }
  }
  _maxBCID_histo->Fill(_maxBCID);
  _maxBCID_histozoom->Fill(_maxBCID);
  _triggerPerReadout->Fill(BCIDwithTrigger.size());
  for (auto d:BCIDwithTriggerPerMezzanine)
    {
      _triggerPerReadoutPerMezzanine->Fill(d.second.size(),d.first);
#ifdef LAURENT_STYLE
      if (d.second.size()>1)
	{
	  TdcChannel* rem=std::remove_if(tdcBuf.begin(), tdcBuf.end(), TdcMezzaninePredicate(d.first) );
	  tdcBuf.setEnd(rem);
	}
#endif
    }
  //eventually here put a filter on the  BCIDwithTrigger set (like remove first ones, last ones, close ones)

  TdcChannel* eventStart=tdcBuf.begin();
  TdcChannel* eventEnd=nullptr;
  //std::cout << "Nombre de triggers = " << BCIDwithTrigger.size() << std::endl;
  for (std::set<std::pair<uint16_t,double>>::iterator it=BCIDwithTrigger.begin(); it !=BCIDwithTrigger.end(); ++it)
  {
    eventEnd=std::partition(eventStart,tdcBuf.end(),TdcChannelBcidpredicate((*it).first,(*it).second,-6,-3));
    processTrigger(eventStart,eventEnd);
    eventStart=eventEnd;
  }
  if(BCIDwithTrigger.size()==0)processNoise(eventStart,tdcBuf.end());
}


void ReadoutProcessor::processTrigger(TdcChannel* begin,TdcChannel* end)
{ 
  TdcChannel* mezzStart=begin;
  TdcChannel* mezzEnd=nullptr;
  for(std::map<int,int>::iterator it=IPtoChamber.begin();it!=IPtoChamber.end();++it)
  {
    mezzEnd=std::partition(mezzStart,end,TdcMezzaninePredicate(it->first));
	  processMezzanine(mezzStart,mezzEnd);
	  mezzStart=mezzEnd;
  }
  //if (nullptr != mezzEnd && end != mezzEnd) std::cout << "WARNING WARNING mess here" << std::endl;
  _counters.newSet();
}

void ReadoutProcessor::processNoise(TdcChannel* begin,TdcChannel* end)
{
  data.Reset();
  for (TdcChannel* it=begin; it != end; ++it)
  {
    data.Push_back(it->side(),it->strip(),it->mezzanine(),it->tdcTime());
  }
  data.OneNoise();
  noiseTree->Fill();
}

bool isTrigger(TdcChannel& c) {return c.channel()==triggerChannel;}

void ReadoutProcessor::processMezzanine(TdcChannel* begin,TdcChannel* end)
{
  data.Reset();
  int trigCount=std::count_if(begin,end,isTrigger);
  if (trigCount != 1) return;
  //std::cout << " trigCount pour nhit = " << int(end-begin) << std::endl;
  TdcChannel *trigger=std::find_if(begin,end,isTrigger);
  unsigned int valeur[2]={(unsigned int)trigger->chamber(),(unsigned int)trigger->mezzanine()};
  //std::cout<<trigger->chamber()<<std::endl;
  for (TdcChannel* it=begin; it != end; ++it) 
  {
    if(it->channel()!=triggerChannel)
    {
      std::cout<<std::setprecision (std::numeric_limits<double>::digits10+1)<<it->tdcTime()-trigger->tdcTime()<<std::endl;
      data.Push_back(it->side(),it->strip(),it->mezzanine(),it->tdcTime(),trigger->tdcTime());
    }
  }
  std::cout<<trigger->chamber()<<std::endl;
  data.OneEvent();
  dataTree->Fill();

  int to_add=0;
  if (int(end-begin)>1) //at least one hit more than the trigger
  {
    TdcChannel* endTrigWindow=std::remove_if(begin,end,TdcOutofTriggerTimePredicate(trigger->tdcTime(),-900,-861));
    if (endTrigWindow!=begin) to_add=1;
  }
  _counters.add(to_add,valeur);
}
