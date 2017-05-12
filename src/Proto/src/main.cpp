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
#include "TH1F.h"
#include "TH2F.h"

const float vincopper=2.*29.979245800/3; //cm.ns-1
const unsigned int Zone=5;
std::array<std::array<int,32>,2>TDCchannelToStrip
{{
  {15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0},
  {16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31}
}};
std::map<int,int>IPtoChamber{{14,0},{15,0}};

std::map<int,int>ReferenceChannel{{0,5}};

RAWDataTriggered data;
RAWDataNotTriggered noise;
TTree* dataTree=nullptr;
TTree* noiseTree=nullptr;
TH1F* numbertrigger= new TH1F("number of triggers in event","number of triggers in event",20,0,20);
TH1F* numbertriggerselected= new TH1F("number of triggers selected in event","number of triggers selected in event",20,0,20);
TH1F* maxtimee= new TH1F("Max time","Max time",200,0,200);

std::map<int,TH1F*> T1mT2;
std::map<int,TH1F*> Calib;
std::map<int,TH2F*> Position;

int readstream(int32_t _fdIn)
{
  if (_fdIn<=0) return 0;
  uint32_t _event=0;
  levbdim::buffer b(0x100000);
  while (true)
  {
    std::map<int,std::vector<TdcChannel>> HitsCloseToTrigger;
    std::map<int,std::vector<TdcChannel>> HitsInMezzanine;
    std::map<int,std::vector<TdcChannel>> TriggerInMezzanine;
    TriggerInMezzanine.clear();
    HitsInMezzanine.clear();
    HitsCloseToTrigger.clear();
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
    int maxtime=-1;
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
	      uint8_t* cbuf=( uint8_t*)&ibuf[7];
	      for (int i=0;i<nch;i++)
		    {
		      TdcChannel c(&cbuf[8*i]);
		      c.setstrip(TDCchannelToStrip[ibuf[4]-1][c.channel()]+100*IPtoChamber[(ibuf[5]>>24)&0xFF]);
		      c.setside(ibuf[4]%2);
		      if((int)c.channel()==28)
		      {
		        TriggerInMezzanine[((ibuf[5]>>24)&0xFF)*100+ibuf[4]].push_back(c);
		      }
		      else HitsInMezzanine[((ibuf[5]>>24)&0xFF)*100+ibuf[4]].push_back(c);
		      if(c.tdcTime()>maxtime)maxtime=c.tdcTime();
		    }
		  }
		}
		std::map<int,std::vector<TdcChannel>> SelectedTriggerInMezzanine;
		int totalTrigger=0;
		for(std::map<int,std::vector<TdcChannel>>::iterator it=TriggerInMezzanine.begin();it!=TriggerInMezzanine.end();++it)
		{
		  totalTrigger+=(it->second).size();
		  if((it->second).size()!=0)
		  {
		      if((it->second).size()==1&&(it->second)[0].tdcTime()>=3*2e7)SelectedTriggerInMezzanine[it->first].push_back((it->second)[0]);
	        for(unsigned int y=0;y!=(it->second).size()-1;++y)
		      {
		        if(std::fabs((it->second)[y+1].tdcTime()-(it->second)[y].tdcTime())>=6*2e7) 
		        {
		          if(y==0&&(it->second)[y].tdcTime()>=3*2e7)SelectedTriggerInMezzanine[it->first].push_back((it->second)[y]);
		          else SelectedTriggerInMezzanine[it->first].push_back((it->second)[y]);
		        }
		        if (y==(it->second).size()-2&&fabs((it->second)[(it->second).size()-1].tdcTime()-maxtime)>=3*2e7)
		        {
		          SelectedTriggerInMezzanine[it->first].push_back((it->second)[(it->second).size()-1]);
		        }
		        else y++;
		      }
		  }
		}
		numbertrigger->Fill(totalTrigger);
		maxtimee->Fill(maxtime*1e-9);
		int selectedtotalTrigger=0;
		for(std::map<int,std::vector<TdcChannel>>::iterator it=SelectedTriggerInMezzanine.begin();it!=SelectedTriggerInMezzanine.end();++it)
		{
		  selectedtotalTrigger+=(it->second).size();
		}
		numbertriggerselected->Fill(selectedtotalTrigger);
	  if(selectedtotalTrigger!=0)
		{
		  for(std::map<int,std::vector<TdcChannel>>::iterator it=HitsInMezzanine.begin();it!=HitsInMezzanine.end();++it)
		  {
		    for(unsigned int i=0;i!=(it->second).size();++i) 
		    {
		      if(SelectedTriggerInMezzanine.find(it->first)!=SelectedTriggerInMezzanine.end())
		      {
		        for(unsigned int j=0;j!=SelectedTriggerInMezzanine[it->first].size();++j) 
		        {
		          data.Reset();
		          data.OneEvent();
		          if(fabs(SelectedTriggerInMezzanine[it->first][j].tdcTime()-(it->second)[i].tdcTime())<=3*2e7)
		          {
		            std::cout<<green<< "    >>>  " << (it->second)[i]<<normal << std::endl;
		            data.Push_back((int)((it->second)[i].strip()),float(((it->second)[i].tdcTime())*1.0e-9),float((SelectedTriggerInMezzanine[it->first][j].tdcTime())*1.0e-9));
		            (it->second)[i].settimefromtrigger((((it->second)[i].tdcTime())-SelectedTriggerInMezzanine[it->first][j].tdcTime())*1.0e-9);
		            HitsCloseToTrigger[IPtoChamber[it->first/100]].push_back((it->second)[i]);
		          }
		          dataTree->Fill();
		        }
		        std::cout<<std::endl;
		      }
		      else 
		      {
		        data.Reset();
		        data.OneEvent();
		        for(std::map<int,std::vector<TdcChannel>>::iterator it=HitsInMezzanine.begin();it!=HitsInMezzanine.end();++it)
		        {
		          for(unsigned int i=0;i!=(it->second).size();++i) 
		          {
		            std::cout<<green<< "    >>>  " << (it->second)[i]<<normal << std::endl;
		            data.Push_back((int)((it->second)[i].strip()),float(((it->second)[i].tdcTime())*1.0e-9),float(0.));
		            (it->second)[i].settimefromtrigger((((it->second)[i].tdcTime())-0.)*1.0e-9);
		            HitsCloseToTrigger[IPtoChamber[it->first/100]].push_back((it->second)[i]);
		          }
		        }
		        dataTree->Fill();
		      }
		    }
		  }
		}
		else 
		{
		  noise.Reset();
		  noise.OneEvent();
		  for(std::map<int,std::vector<TdcChannel>>::iterator it=HitsInMezzanine.begin();it!=HitsInMezzanine.end();++it)
		  {
		    for(unsigned int i=0;i!=(it->second).size();++i) 
		    {
		      std::cout<<yellow << "    >>>  " << (it->second)[i]<<normal << std::endl;
		      noise.Push_back((int)((it->second)[i].strip()),float(((it->second)[i].tdcTime())*1.0e-9));
		    }
		  }
      noiseTree->Fill();
		}
    std::cout<<normal<<std::endl;
    std::cout<<std::endl;
    for(std::map<int,std::vector<TdcChannel>>::iterator it=HitsCloseToTrigger.begin();it!=HitsCloseToTrigger.end();++it)
    {
      std::cout<<blue<<"Hits close to trigger in detector and touch in both sides"<<it->first<<" : "<<normal<<std::endl;
      bool referenceistouched=false;
      std::vector<std::pair<int,float>>T1mT2ReferenceStrip;
      std::vector<std::pair<int,float>>T1mT2Strip;
      for(unsigned g=0;g!=it->second.size();++g)
      {
        for(unsigned h=g;h!=it->second.size();++h)
        {
          if(it->second[h].strip()==it->second[g].strip()&&it->second[h].side()!=it->second[g].side())
          {
            if(ReferenceChannel.find(it->first)!=ReferenceChannel.end())
            {
              if(ReferenceChannel[it->first]==(it->second)[h].strip()) 
              {
                referenceistouched=true;
                if((it->second)[h].side()==1)
                {
                  T1mT2ReferenceStrip.push_back(std::pair<int,float>((it->second)[h].strip(),(it->second)[h].timefromtrigger()));
                }
                else T1mT2ReferenceStrip.push_back(std::pair<int,float>((it->second)[g].strip(),(it->second)[g].timefromtrigger()));
              }
              else if (fabs((it->second)[h].strip()-(it->second)[g].strip())<=Zone)
              {
                if((it->second)[h].side()==1)
                {
                  T1mT2ReferenceStrip.push_back(std::pair<int,float>((it->second)[h].strip(),(it->second)[h].timefromtrigger()));
                }
                else T1mT2ReferenceStrip.push_back(std::pair<int,float>((it->second)[g].strip(),(it->second)[g].timefromtrigger()));
              }
            }
            std::cout<<blue<< "****** " << (it->second)[g]<<normal << std::endl;
            std::cout<<blue<< "****** " << (it->second)[h]<<normal << std::endl;
            std::cout<<red<<(it->second)[h].timefromtrigger()-(it->second)[g].timefromtrigger()<<normal<<std::endl;
            float diff=(it->second)[h].timefromtrigger()-(it->second)[g].timefromtrigger();
            if((it->second)[h].side()==1) diff=-diff;
            T1mT2[(it->first*100+(it->second)[h].strip())]->Fill(diff);
            Position[it->first]->Fill((it->second)[h].strip(),(100-vincopper*diff)*1.0/2);
          }
        }
      }
      if(referenceistouched==true)
      {
        for(unsigned g=0;g!=T1mT2ReferenceStrip.size();++g)
        {
          for(unsigned h=0;h!=T1mT2Strip.size();++h)
          {
            Calib[(it->first*100+T1mT2Strip[h].first)]->Fill(T1mT2Strip[h].second-T1mT2ReferenceStrip[g].second);
          }
        }
      }
    }
    HitsCloseToTrigger.clear();
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
  std::size_t found = filename.rfind(".");
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
  for(std::map<int,int>::iterator it=IPtoChamber.begin();it!=IPtoChamber.end();++it)
  {
    if(Position.find(it->second)==Position.end())
    {
      std::ostringstream ostr;
      ostr<<"Position_hits_Chamber"<<it->second;
      Position[it->second]=new TH2F((ostr.str()).c_str(),(ostr.str()).c_str(),32,0,32,1000,-500,500);
    }
    if(T1mT2.find(it->second)==T1mT2.end())
    {
      for(unsigned int i=0;i!=32;++i)
      {
         std::ostringstream ostr;
         ostr<<"T_{1}-T_{2}_Chamber"<<it->second<<"_Channel"<<i;
         T1mT2[it->second*100+i]=new TH1F((ostr.str()).c_str(),(ostr.str()).c_str(),2000,0,2000);
      }
    }
    if(Calib.find(it->second)==Calib.end())
    {
      if(ReferenceChannel.find(it->second)!=ReferenceChannel.end()) 
      {
        int before=ReferenceChannel[it->second]-Zone;
        int after =ReferenceChannel[it->second]+Zone;
        std::cout<<red<<"tttttttttttttttttttttttttttttttttttttttttttt "<<before<<"   "<<after<<normal<<std::endl;
        if (before<0) before=0;
        if (after>32) after=31;
        for(unsigned int i=before;i!=after+1;++i)
        {
         if(i==ReferenceChannel[it->second]) continue;
         std::ostringstream ostr;
         ostr<<"Calib_Chamber"<<it->second<<"_Channel"<<i<<"_Reference_Channel"<<ReferenceChannel[it->second];
         Calib[it->second*100+i]=new TH1F((ostr.str()).c_str(),(ostr.str()).c_str(),2000,0,2000);
        }
      }
    }
  }
  TBranch *bEventNumberData = dataTree->Branch("EventNumber",  &data.iEvent,1000,0);
  TBranch *bNumberOfHitsData = dataTree->Branch("number_of_hits", &data.TDCNHits,1000,0);
  TBranch *bTDCChannelData = dataTree->Branch("TDC_channel",  &data.TDCCh,1000,0);
  TBranch *bTDCTimeStampData = dataTree->Branch("TDC_TimeStamp", &data.TDCTS,1000,0);
  TBranch *bTDCTimeStampDataReal = dataTree->Branch("TDC_TimeStampReal", &data.TDCTSReal,1000,0);
  TBranch *bWitchSideData = dataTree->Branch("WichSide",  &data.WitchSide,1000,0);
  TBranch *bEventNumberNoise = noiseTree->Branch("EventNumber_without_trigger",  &noise.iEvent,1000,0);
  TBranch *bNumberOfHitsNoise = noiseTree->Branch("number_of_hits_without_trigger", &noise.TDCNHits,1000,0);
  TBranch *bTDCChannelNoise = noiseTree->Branch("TDC_channel_without_trigger",  &noise.TDCCh,1000,0);
  TBranch *bTDCTimeStampNoise = noiseTree->Branch("TDC_TimeStamp_without_trigger", &noise.TDCTS,1000,0);
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
  numbertrigger->Write();
  numbertriggerselected->Write();
  TDirectory *cdtof = file.mkdir("T1mT2");
  cdtof->cd();    // make the "tof" directory the current directory
  for(std::map<int,TH1F*>::iterator it=T1mT2.begin();it!=T1mT2.end();++it)
  {
    (it->second)->Write();
    delete (it->second);
  }
  T1mT2.clear();
  file.cd();
  TDirectory *cdtof2 = file.mkdir("Hits position");
  cdtof2->cd();    // make the "tof" directory the current directory
  for(std::map<int,TH2F*>::iterator it=Position.begin();it!=Position.end();++it)
  {
    (it->second)->Write();
    delete (it->second);
  }
  Position.clear();
  file.cd();
  TDirectory *cdtof3 = file.mkdir("Calib");
  cdtof3->cd();    // make the "tof" directory the current directory
  for(std::map<int,TH1F*>::iterator it=Calib.begin();it!=Calib.end();++it)
  {
    (it->second)->Write();
    delete (it->second);
  }
  Calib.clear();
  file.cd();
  maxtimee->Write();
  delete numbertrigger;
  delete numbertriggerselected;
  delete maxtimee;

  //delete bEventNumberData;
  //delete bNumberOfHitsData;
  //delete bTDCChannelData;
  //delete bTDCTimeStampData;
  //delete bEventNumberNoise;
  //delete bNumberOfHitsNoise;
  //delete bTDCChannelNoise;
  //delete bTDCTimeStampNoise;
  //delete dataTree;
  //delete noiseTree;
  file.Close();
  return retur;
}

