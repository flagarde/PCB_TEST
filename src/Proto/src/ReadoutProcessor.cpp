#include "ReadoutProcessor.h" 
#include <set>
#include <algorithm>
#include "Global.h"
#include "Predicate.h"
#include "buffer.h"
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
  TdcChannel* eventEnd=nullptr;
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
  TdcChannel* mezzEnd=nullptr;
  for(std::map<int,int>::iterator it=IPtoChamber.begin();it!=IPtoChamber.end();++it)
  {
    mezzEnd=std::partition(mezzStart,end,TdcChannelMezzaninePredicate(it->second,it->first));
	  processMezzanine(mezzStart,mezzEnd);
	  mezzStart=mezzEnd;
  }
  //if (nullptr != mezzEnd && end != mezzEnd) std::cout << "WARNING WARNING mess here" << std::endl;
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
  std::cout<<trigger->chamber()<<std::endl;
  if (int(end-begin)>1) //at least one hit more than the trigger
  {
      _counters.add(1,valeur);
  }
  else _counters.add(0,valeur);
}
