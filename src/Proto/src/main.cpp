// g++ main.cpp -lz
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <fcntl.h>
#include <iostream>
#include <limits>
#include "buffer.hh"
#include "TdcChannels.hh"
#include "Colors.h"
#include "TFile.h"
#include "TTree.h"
#include "RAWData.h"


std::map<int,int>IPtoChamber{{14,0},{15,0}};

RAWDataTriggered data;
RAWDataNotTriggered noise;
TTree* dataTree=nullptr;
TTree* noiseTree=nullptr;

int readstream(int32_t _fdIn)
{
  if (_fdIn<=0) return 0;
  uint32_t _event=0;
  levbdim::buffer b(0x100000);
  while (true)
  {
    uint32_t theNumberOfDIF=0;
    int ier=::read(_fdIn,&_event,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore %d \n ",ier); return 0;
	  }
    else printf("Event read %d \n",_event);
    ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore number of DIF %d \n ",ier); return 0;
	  }
    else printf("Number of DIF found %d \n",theNumberOfDIF);
    bool hasnoise=false;
    bool hasdata=false;
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
	    printf("\t \t %d %d %d %lu %d %d\n",b.detectorId(),b.dataSourceId(),b.eventId(),b.bxId(),b.payloadSize(),bsize);
	    if (b.detectorId() != 110) continue;
      uint32_t* ibuf=(uint32_t*) b.payload();
	    uint64_t absbcid=ibuf[3]; absbcid=(absbcid<<32)|ibuf[2];
	    printf("\n event number %d, GTC %d, ABSBCID %lu, mezzanine number %u, ",ibuf[0],ibuf[1]&0xFFFF,absbcid,ibuf[4]);
	    printf("IP address %u.%u.%u.%u,",ibuf[5]&0xFF,(ibuf[5]>>8)&0xFF,(ibuf[5]>>16)&0xFF,(ibuf[5]>>24)&0xFF);
	    uint32_t nch=ibuf[6];
	    printf("\n channels -> %d \n",nch);
	    if (nch>0)
	    {
	      std::vector<TdcChannel> _channels;
	      _channels.reserve(nch);
	      bool hasTrigger=false;
	      float triggertime=0;
	      uint8_t* cbuf=( uint8_t*)&ibuf[7];
	      for (int i=0;i<nch;i++)
		    {
		      TdcChannel c(&cbuf[8*i]);
		      if((int)c.channel()==28)
		      {
		        hasTrigger=true;
		        triggertime=c.tdcTime();
		      }
		      else _channels.push_back(c);
		      //std::cout << "    >>>  " << c << std::endl;
		    }
		    if(hasTrigger==true)
		    {
		      hasdata=true;
		      for(unsigned int i=0;i!=_channels.size();++i) 
		      {
		        std::cout<<green << "    >>>  " << _channels[i]<<normal << std::endl;
		        data.Reset();
		        data.Reserve(_channels.size());
		        data.Push_back(ibuf[4],_channels[i].channel()+1000*IPtoChamber[(ibuf[5]>>24)&0xFF],_channels[i].tdcTime()*1.0e-9,triggertime*1.0e-9);
		      }
		    }
		    else 
		    {
		      hasnoise=true;
		      for(unsigned int i=0;i!=_channels.size();++i) 
		      {
		        std::cout<<yellow << "    >>>  " << _channels[i]<<normal << std::endl;
		        noise.Reset();
		        noise.Reserve(_channels.size());
		        noise.Push_back(ibuf[4],_channels[i].channel()+1000*IPtoChamber[(ibuf[5]>>24)&0xFF],_channels[i].tdcTime()*1.0e-9);
		      }
		    }
		    std::cout<<normal<<std::endl;
	    }
	  } // end loop on DIF
    if(hasdata==true)
    {
      data.OneEvent();
      dataTree->Fill();
    }
    if(hasnoise==true)
    {
      noise.OneEvent();
      noiseTree->Fill();
    }
  }
}

int main(int argc, char *argv[])
{
  if(argc!=2)
  {
    std::cout<<"Please provide the file you want to convert"<<std::endl;
    std::exit(2);
  }
  std::string filename=argv[1];
  std::size_t found = filename.find(".");
  std::string filena=filename;
  filena=filena.erase(found);
  TFile file((filena+".root").c_str(),"RECREATE",filena.c_str(),9);
  if (file.IsOpen() != true) 
  {
    std::cout << red << "Impossible to open " << filena<< normal << std::endl;
    std::exit(1);
  }
  dataTree=new TTree("RAWData","RAWData"); 
  noiseTree=new TTree("RAWNoise","RAWNoise"); 
  if (!dataTree) 
  {
    std::cout << red << "Impossible to create TTree RAWData" << normal<< std::endl;
    std::exit(1);
  }
   if (!noiseTree) 
  {
    std::cout << red << "Impossible to create TTree RAWData" << normal<< std::endl;
    std::exit(1);
  }
  TBranch *bEventNumberData = dataTree->Branch("EventNumber",  &data.iEvent,1000,0);
  TBranch *bNumberOfHitsData = dataTree->Branch("number_of_hits", &data.TDCNHits,1000,0);
  TBranch *bTDCChannelData = dataTree->Branch("TDC_channel",  &data.TDCCh,1000,0);
  TBranch *bTDCTimeStampData = dataTree->Branch("TDC_TimeStamp", &data.TDCTS,1000,0);
  TBranch *bTDCTimeStampDataReal = dataTree->Branch("TDC_TimeStampReal", &data.TDCTSReal,1000,0);
  TBranch *bEventNumberNoise = noiseTree->Branch("EventNumber_without_trigger",  &noise.iEvent,1000,0);
  TBranch *bNumberOfHitsNoise = noiseTree->Branch("number_of_hits_without_trigger", &noise.TDCNHits,1000,0);
  TBranch *bTDCChannelNoise = noiseTree->Branch("TDC_channel_without_trigger",  &noise.TDCCh,1000,0);
  TBranch *bTDCTimeStampNoise = noiseTree->Branch("TDC_TimeStamp_without_trigger", &noise.TDCTS,1000,0);
  TBranch *bWitchSideData = dataTree->Branch("WichSide",  &data.WitchSide,1000,0);
  TBranch *bWitchSideNoise = noiseTree->Branch("WichSide_without_trigger", &noise.WitchSide,1000,0);
  int32_t _fdIn=::open(filename.c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
  if (_fdIn<0)
  {
    perror("Can't open file :");
    return 1;
  }
  int retur=readstream(_fdIn);
  dataTree->Write();
  noiseTree->Write();
  delete dataTree;
  delete noiseTree;
  //delete bEventNumberData;
  //delete bNumberOfHitsData;
  //delete bTDCChannelData;
  //delete bTDCTimeStampData;
  //delete bEventNumberNoise;
  //delete bNumberOfHitsNoise;
  //delete bTDCChannelNoise;
  //delete bTDCTimeStampNoise;
  file.Close();
  return retur;
}

