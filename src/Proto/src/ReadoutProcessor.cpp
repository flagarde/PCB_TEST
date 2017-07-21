#include "ReadoutProcessor.h" 
#include <set>
#include <algorithm>
#include "Global.h"
#include "Predicate.h"
#include "buffer.h"
#include "TF1.h"
#include "Plots.h"
#include "Clustering.h"
#include "RawHit_standard_merge_predicate.h"
#include <fstream>
#include "TSpectrum.h"


//estimate linear background using a fitting method
std::set<double> findPeaks(TH1* obj)
{
  std::set<double> xs;
  TSpectrum *s = new TSpectrum(20);
  Int_t nfound = s->Search(obj,3,"",0.05);
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  Double_t *xpeaks = s->GetPositionX();
#else
  Float_t *xpeaks = s->GetPositionX();
#endif
  for(int p=0;p<nfound;p++) 
  {
    xs.insert(xpeaks[p]);
  }
  delete s;
  return xs;
}
 
//#define LAURENT_STYLE
int ReadoutProcessor::readstream(int32_t _fdIn,bool firstTime)
{
  if (_fdIn<=0) return 0;
  uint32_t _event=0;
  uint32_t maxsize=0x100000;
  levbdim::buffer b(maxsize);
  TdcChannelBuffer tdcBuf(maxsize);
  tdcBuf.setIsNoise(false);
  while (true)
  {
    tdcBuf.clear();
    uint32_t theNumberOfDIF=0;
    int ier=::read(_fdIn,&_event,sizeof(uint32_t));
    _totalevent+=1;
    if (ier<=0)
	  {
	    printf("Cannot read anymore %d \n",ier); 
	    return 0;
	  }
    else if(_totalevent%10000==0) printf("Event read %d \n",_totalevent);
    if(_numbereventtoprocess<=_totalevent&&_numbereventtoprocess!=-1)return 2;
    ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore number of DIF %d \n ",ier); 
	    return 0;
	  }
    //else printf("Number of DIF found %d \n",theNumberOfDIF);
    unsigned int nDIFwithChannel=0;
    std::set<uint64_t> absbcidFoundInThisReadout;
    bool OKprocess=true;
    uint64_t absbcid=0;
    for (uint32_t idif=0;idif<theNumberOfDIF;idif++) 
	  {
	    uint32_t bsize=0;
	    ier=::read(_fdIn,&bsize,sizeof(uint32_t));
	    if (ier<=0)
	    {
	      printf("Cannot read anymore  DIF Size %d \n ",ier);
	      return 0;
	    }
	    ier=::read(_fdIn,b.ptr(),bsize);
	    if (ier<=0)
	    {
	      printf("Cannot read anymore Read data %d \n ",ier);
	      return 0;
	    }
	    b.setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
	    b.uncompress();
	    if (b.detectorId() != 110) continue;
	    uint32_t* ibuf=(uint32_t*) b.payload();
	    absbcid=ibuf[3]; absbcid=(absbcid<<32)|ibuf[2];
	    if (ibuf[6]>0)
	    {
	      ++nDIFwithChannel;
	      absbcidFoundInThisReadout.insert(absbcid);
	      uint8_t* cbuf=( uint8_t*)&ibuf[7];
	      for (int i=0;i<ibuf[6];i++)
		    {
		      tdcBuf.addChannel(&cbuf[8*i]);
		      TdcChannel &c=tdcBuf.last();
          if (tdcBuf.nTdcChannel()>maxsize) 
          {
            std::cout << "WARNING TOO BIG EVENT skipping" << std::endl; 
            OKprocess=false; 
            break;
          }
          if(c.channel()==triggerChannel)
          {
            _lastTriggerAbsBCID=absbcid;
          }
		      c.setstrip(ibuf[4],(ibuf[5]>>24)&0xFF);
		    }
	    }
	  } // end loop on DIF
    _nDIFinReadout->Fill(nDIFwithChannel);
    for (auto it=absbcidFoundInThisReadout.begin(); it != absbcidFoundInThisReadout.end(); ++it) _AbsBCID_Readout_map[*it]++;
    if(tdcBuf.nTdcChannel()>nbrTotalHitsMax) return 0;
    if (OKprocess) 
    {
      if(_lastTriggerAbsBCID!=0)
      {
        if(absbcid>=_lastTriggerAbsBCID+FourSecondsInClockTicks&& absbcid<=_lastTriggerAbsBCID+TenSecondsInClockTicks)
        {
          //std::cout<<_lastTriggerAbsBCID+FourSecondsInClockTicks<<"  "<< absbcid<<"  "<<_lastTriggerAbsBCID+TenSecondsInClockTicks<<std::endl;
          tdcBuf.setIsNoise(true);
        }
        //else std::cout<<red<<_lastTriggerAbsBCID+FourSecondsInClockTicks<<"  "<< absbcid<<"  "<<_lastTriggerAbsBCID+TenSecondsInClockTicks<<normal<<std::endl;
      }
      processReadout(tdcBuf,firstTime);
    }
  }
} 


void ReadoutProcessor::init()
{
   for(unsigned int i=0;i!=3;++i) _ugly[i].reserve(500);
  _myfile.open("Results_Effi.txt",std::ios::out|std::ios::app);
  _myfilestreamer.open("Results_Streamer.txt",std::ios::out|std::ios::app);
  _myfilepeaks.open("Results_Peaks.txt",std::ios::out|std::ios::app);
  _maxBCID_histo= new TH1F("MAX_BCID","Maximum BCID",200,0,200);
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
  _maxBCID_histo->GetXaxis()->SetCanExtend(true);
#else
  _maxBCID_histo->SetBit(TH1::kCanRebin);
#endif
  _maxBCID_histozoom= new TH1F("MAX_BCID_zoom","Maximum BCID",200,0,200);
  _triggerPerReadout=new TH1F("triggerPerReadout","number of triggers per readout",50,0,50);
  _triggerPerReadoutPerMezzanine=new TH2F("triggerPerReadoutPerMezzanine","number of triggers per readout per mezzanine",50,0,50,4,12,16);
  _triggerPerReadoutPerMezzanine->GetXaxis()->SetTitle("Number of trigger in readout");
  _triggerPerReadoutPerMezzanine->GetYaxis()->SetTitle("Mezzanine");
  _triggerTime=new TH1F("triggerTime","Time of hits minus triggerTime",20000,-20000,0);
  _nDIFinReadout=new TH1F("nDIFinReadout","Number of DIF with channel found in event",10,0,10);
  _nReadoutperAbsBCID=new TH1F("nReadoutperAbsBCID","Number of readout per absolute BCID",10,0,10);
  _noisehitspersecond=new TProfile("Hits_noise_rate","Hits noise rate",20,0,20);
  _data.Reserve(1000);
  _dataTree=new TTree("RAWData","RAWData"); 
  _noiseTree=new TTree("RAWNoise","RAWNoise"); 
  _bEventNumber = _dataTree->Branch("EventNumber",  &_data.iEvent,50000,0);
  _bNumberOfHits = _dataTree->Branch("number_of_hits", &_data.TDCNHits,50000,0);
  _bTDCChannel = _dataTree->Branch("TDC_channel",  &_data.TDCCh,50000,0);
  _bTDCTimeStamp = _dataTree->Branch("TDC_TimeStamp", &_data.TDCTS,50000,0);
  _bTDCTimeStampReal = _dataTree->Branch("TDC_TimeStampReal", &_data.TDCTSReal,50000,0);
  _bWitchSide = _dataTree->Branch("WichSide",  &_data.WitchSide,50000,0);
  _bMezzanine = _dataTree->Branch("Mezzanine",  &_data.Mezzanine,50000,0);
  _bEventNumber2 = _noiseTree->Branch("EventNumber",  &_data.iNoise,50000,0);
  _bNumberOfHits2 = _noiseTree->Branch("number_of_hits", &_data.TDCNHits,50000,0);
  _bTDCChannel2 = _noiseTree->Branch("TDC_channel",  &_data.TDCCh,50000,0);
  _bTDCTimeStampReal2 = _noiseTree->Branch("TDC_TimeStampReal",&_data.TDCTSReal,50000,0);
  _bWitchSide2 = _noiseTree->Branch("WichSide",  &_data.WitchSide,50000,0);
  _bMezzanine2 = _noiseTree->Branch("Mezzanine",  &_data.Mezzanine,50000,0);
  for(std::map<int,int>::iterator it=IPtoChamber.begin();it!=IPtoChamber.end();++it)
  {
     _hitTimePair[it->first]=new TH1F(("triggerTimePair_mezz"+std::to_string(it->first)).c_str(),("Time of hits minus triggerTime even strip mezzanine "+std::to_string(it->first)).c_str(),2000,-2000,0);
     _hitTimeImpair[it->first]=new TH1F(("triggerTimeImpair_mezz"+std::to_string(it->first)).c_str(),("Time of hits minus triggerTime odd strip mezzanine "+std::to_string(it->first)).c_str(),2000,-2000,0);
    if(_T1mT2.find(it->second)==_T1mT2.end())
    {
        _tmt0[it->second]=new TH1F(("T-T0_"+std::to_string(it->second)).c_str(),("T-T0_"+std::to_string(it->second)).c_str(),2000,-2000,0);
        _t1mt0[it->second]=new TH1F(("T1-T0_"+std::to_string(it->second)).c_str(),("T1-T0_"+std::to_string(it->second)).c_str(),2000,-2000,0);
        _t2mt0[it->second]=new TH1F(("T2-T0_"+std::to_string(it->second)).c_str(),("T2-T0_"+std::to_string(it->second)).c_str(),2000,-2000,0);
       _Correlation[it->second]=new TH2F(("Correlation_"+std::to_string(it->second)).c_str(),("Correlation"+std::to_string(it->second)).c_str(),32,0,32,32,0,30);
      _T1mT2[it->second]=new TH2F(("T1-T2_th2_"+std::to_string(it->second)).c_str(),("T1-T2_th2_"+std::to_string(it->second)).c_str(),32,0,32,10000,-500,500);
      _Position[it->second]=new TH2F(("Position_"+std::to_string(it->second)).c_str(),("Position_"+std::to_string(it->second)).c_str(),32,0,32,20000,-1000,1000); 
      _Longueur[it->second]=new TH2F(("Longueur_"+std::to_string(it->second)).c_str(),("Longueur_"+std::to_string(it->second)).c_str(),32,0,32,20000,-1000,1000);
      _T1mT2Chamber[it->second]=new TH1F(("T1-T2_"+std::to_string(it->second)).c_str(),("T1-T2_"+std::to_string(it->second)).c_str(),10000,-500,500);
      _MultiplicitySide0[it->second]=new TH1F(("Hit_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),("Hit_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),60,0,60);
      _MultiplicitySide1[it->second]=new TH1F(("Hit_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),("Hit_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),60,0,60);
      _MultiplicityBothSide[it->second]=new TH1F(("Hit_Multiplicity_BothSide_"+std::to_string(it->second)).c_str(),("Hit_Multiplicity_BothSide_"+std::to_string(it->second)).c_str(),60,0,60);
      _NbrClusterSide0[it->second]=new TH1F(("Number_of_Cluster_Side0_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Side0_"+std::to_string(it->second)).c_str(),10,0,10);
      _MultiClusterSide0[it->second]=new TH1F(("Cluster_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),60,0,60);
      _NbrClusterSide1[it->second]=new TH1F(("Number_of_Cluster_Side1_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Side1_"+std::to_string(it->second)).c_str(),10,0,10);
      _MultiClusterSide1[it->second]=new TH1F(("Cluster_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),60,0,60);
      _NbrClusterBothSide[it->second]=new TH1F(("Number_of_Cluster_Both_Side_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Both_Side_"+std::to_string(it->second)).c_str(),100,0,100);
      _MultiClusterBothSide[it->second]=new TH1F(("Cluster_Multiplicity_Both_Side_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Both_Side_"+std::to_string(it->second)).c_str(),60,0,60);
       //NOISE
      _NbrClusterNoiseSide0[it->second]=new TH1F(("Number_of_Noise_Cluster_Side0_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Side0_"+std::to_string(it->second)).c_str(),100,0,100);
      _MultiClusterNoiseSide0[it->second]=new TH1F(("Cluster_Noise_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Side0_"+std::to_string(it->second)).c_str(),60,0,60);
      _NbrClusterNoiseSide1[it->second]=new TH1F(("Number_of_Noise_Cluster_Side1_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Side1_"+std::to_string(it->second)).c_str(),100,0,100);
      _MultiClusterNoiseSide1[it->second]=new TH1F(("Cluster_Noise_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Side1_"+std::to_string(it->second)).c_str(),60,0,60);
      _NbrClusterNoiseBothSide[it->second]=new TH1F(("Number_of_Noise_Cluster_Both_Side_"+std::to_string(it->second)).c_str(),("Number_of_Cluster_Both_Side_"+std::to_string(it->second)).c_str(),100,0,100);
      _MultiClusterNoiseBothSide[it->second]=new TH1F(("Cluster_Noise_Multiplicity_Both_Side_"+std::to_string(it->second)).c_str(),("Cluster_Multiplicity_Both_Side_"+std::to_string(it->second)).c_str(),60,0,60);
      //Cluster timing analysis 
      _NumberOfStripsForT1mT0inCluster[it->second]=new TH1F(("NumberOfStripsForT1mT0inCluster_"+std::to_string(it->second)).c_str(),("Number of strips usable to compute mean of T1-T0 in clusters for chamber "+std::to_string(it->second)).c_str(),32,0,32);
      _NumberOfStripsForT2mT0inCluster[it->second]=new TH1F(("NumberOfStripsForT2mT0inCluster_"+std::to_string(it->second)).c_str(),("Number of strips usable to compute mean of T2-T0 in clusters for chamber "+std::to_string(it->second)).c_str(),32,0,32);
      _NumberOfStripsForT1mT2inCluster[it->second]=new TH1F(("NumberOfStripsForT1mT2inCluster_"+std::to_string(it->second)).c_str(),("Number of strips usable to compute mean of T1-T2 in clusters for chamber "+std::to_string(it->second)).c_str(),32,0,32);
      _MeanT1mimusMeanT2inCluster[it->second]=new TH1F(("MeanT1mimusMeanT2inCluster_"+std::to_string(it->second)).c_str(),("Mean(T1-T0) minus Mean(T2-T0) in clusters for chamber "+std::to_string(it->second)).c_str(),1000,-50,50);
      _MeanT1mimusT2inCluster[it->second]=new TH1F(("MeanT1mimusT2inCluster_"+std::to_string(it->second)).c_str(),("Mean(T1-T2) in clusters for chamber "+std::to_string(it->second)).c_str(),1000,-50,50);
      _MeanT1mimusT2inCluster3StripMin[it->second]=new TH1F(("MeanT1mimusT2inCluster3StripMin_"+std::to_string(it->second)).c_str(),("Mean(T1-T2) in clusters with at least 3 strips for chamber "+std::to_string(it->second)).c_str(),1000,-50,50);
    }
  }
  _clusterTimeAnalysisCut=new TH1F("clusterTimeAnalysisCut","Number of clusters kept",15,0,15);
  // could try to do something complicated with ChambertoIP or IPtoChamber but keep it simple but not portable for now
  _chamberEfficiency.addChamber(14,15);
  _chamberEfficiency.addChamber(12,13);
}


void ReadoutProcessor::finish()
{
  _maxBCID_histo->Write();
  _maxBCID_histozoom->Write();
  _triggerPerReadout->Write();
  _triggerPerReadoutPerMezzanine->Write();
  _triggerTime->Write();
  _nDIFinReadout->Write();
  for (auto it=_AbsBCID_Readout_map.begin(); it !=_AbsBCID_Readout_map.end(); ++it)
    _nReadoutperAbsBCID->Fill(it->second);
  _nReadoutperAbsBCID->Write();
  for (std::map<unsigned int,TH1F*>::iterator it=_hitTimePair.begin();  it != _hitTimePair.end();   ++it) it->second->Write();
  for (std::map<unsigned int,TH1F*>::iterator it=_hitTimeImpair.begin(); it != _hitTimeImpair.end(); ++it) it->second->Write();
  std::string labels[3]={"ALL", "CHAMBER", "MEZZANINE"};
  _myfile<<_nbrRun<<std::endl;
  _counters.write(labels,_myfile);
  _myfile<<std::endl<<std::endl;
  _myfile.close();
  _tdc_counters.write(labels);
  _chamberEfficiency.print();
  _noisehitspersecond->Write();
  _dataTree->Write();
  _noiseTree->Write();
  _folder->cd();
  for(std::map<int,TH1F*>::iterator it=_T1mT2ChOneHit.begin();it!=_T1mT2ChOneHit.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_DistributionHitCloseTrigger.begin();it!=_DistributionHitCloseTrigger.end();++it)
  {
    it->second->Write();
    delete it->second;
  } 
  for(std::map<int,TH1F*>::iterator it=_MultiplicitySide0.begin();it!=_MultiplicitySide0.end();++it)
  {
    it->second->Write();
    delete it->second;
  } 
  for(std::map<int,TH1F*>::iterator it=_MultiplicitySide1.begin();it!=_MultiplicitySide1.end();++it)
  {
    it->second->Write();
    delete it->second;
  } 
  for(std::map<int,TH1F*>::iterator it=_MultiplicityBothSide.begin();it!=_MultiplicityBothSide.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_TimeWithRespectToFirstOneCh2d0.begin();it!=_TimeWithRespectToFirstOneCh2d0.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_TimeWithRespectToFirstOneCh2d1.begin();it!=_TimeWithRespectToFirstOneCh2d1.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it= _TimeWithRespectToFirst.begin();it!= _TimeWithRespectToFirst.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it= _TimeWithRespectToFirstSameStrip0.begin();it!= _TimeWithRespectToFirstSameStrip0.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it= _TimeWithRespectToFirstSameStrip1.begin();it!= _TimeWithRespectToFirstSameStrip1.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_Longueur.begin();it!=_Longueur.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_Correlation.begin();it!=_Correlation.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_Position.begin();it!=_Position.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  _folder->mkdir("Clusters");
  _folder->cd("Clusters");
  for(std::map<int,TH1F*>::iterator it=_NbrClusterSide0.begin();it!=_NbrClusterSide0.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterSide0.begin();it!=_MultiClusterSide0.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_NbrClusterSide1.begin();it!=_NbrClusterSide1.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterSide1.begin();it!=_MultiClusterSide1.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_NbrClusterBothSide.begin();it!=_NbrClusterBothSide.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterBothSide.begin();it!=_MultiClusterBothSide.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  _clusterTimeAnalysisCut->Write();
  for(std::map<int,TH1F*>::iterator it=_NumberOfStripsForT1mT0inCluster.begin();it!=_NumberOfStripsForT1mT0inCluster.end();++it) {it->second->Write();delete it->second;}
  for(std::map<int,TH1F*>::iterator it=_NumberOfStripsForT2mT0inCluster.begin();it!=_NumberOfStripsForT2mT0inCluster.end();++it) {it->second->Write();delete it->second;}
  for(std::map<int,TH1F*>::iterator it=_NumberOfStripsForT1mT2inCluster.begin();it!=_NumberOfStripsForT1mT2inCluster.end();++it) {it->second->Write();delete it->second;}
  for(std::map<int,TH1F*>::iterator it=_MeanT1mimusMeanT2inCluster.begin();it!=_MeanT1mimusMeanT2inCluster.end();++it) {it->second->Write();delete it->second;}
  for(std::map<int,TH1F*>::iterator it=_MeanT1mimusT2inCluster.begin();it!=_MeanT1mimusT2inCluster.end();++it) {it->second->Write();delete it->second;}
  for(std::map<int,TH1F*>::iterator it=_MeanT1mimusT2inCluster3StripMin.begin();it!=_MeanT1mimusT2inCluster3StripMin.end();++it) {it->second->Write();delete it->second;}
  //NOISE
  _folder->mkdir("Clusters_Noise");
  _folder->cd("Clusters_Noise");
    for(std::map<int,TH1F*>::iterator it=_NbrClusterNoiseSide0.begin();it!=_NbrClusterNoiseSide0.end();++it)
  {
    it->second->GetXaxis()->SetTitle("Number of Noise Cluster (Hz.cm^{-2})");
    it->second->GetYaxis()->SetTitle("#");
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterNoiseSide0.begin();it!=_MultiClusterNoiseSide0.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_NbrClusterNoiseSide1.begin();it!=_NbrClusterNoiseSide1.end();++it)
  {
    it->second->GetXaxis()->SetTitle("Number of Noise Cluster (Hz.cm^{-2})");
    it->second->GetYaxis()->SetTitle("#");
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterNoiseSide1.begin();it!=_MultiClusterNoiseSide1.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_NbrClusterNoiseBothSide.begin();it!=_NbrClusterNoiseBothSide.end();++it)
  {
    it->second->GetXaxis()->SetTitle("Number of Noise Cluster (Hz.cm^{-2})");
    it->second->GetYaxis()->SetTitle("#");
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH1F*>::iterator it=_MultiClusterNoiseBothSide.begin();it!=_MultiClusterNoiseBothSide.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  _folder->mkdir("T1-T2");
  _folder->cd("T1-T2");
  TF1 *gauss = new TF1("gauss", "gaus");
  TCanvasDivided a(3,3);
  a.setName("Test_*");
  for(std::map<int,TH1F*>::iterator it=_T1mT2Ch.begin();it!=_T1mT2Ch.end();++it)
  { 
    if(it->second->GetEntries()!=0)
    {
      gauss->SetParameters(it->second->GetMean(),it->second->GetRMS());
      it->second->Fit("gauss","Q");
      TF1 *fit1 = (TF1*)it->second->GetFunction("gaus");
      if(fit1!=nullptr)
      {
        fit1->SetLineColor(kRed);
        fit1->Draw("same");
      }
    }
    a.add(it->second);
    it->second->Write();
  }
  a.write(_folder);
  for(std::map<int,TH1F*>::iterator it=_T1mT2Chamber.begin();it!=_T1mT2Chamber.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  for(std::map<int,TH2F*>::iterator it=_T1mT2.begin();it!=_T1mT2.end();++it)
  {
    it->second->Write();
    delete it->second;
  }
  _folder->mkdir("T1-T0");
  _folder->cd("T1-T0");
  for(std::map<int,TH1F*>::iterator it=_T1mT0Ch.begin();it!=_T1mT0Ch.end();++it)
  { 
    it->second->Write();
  }
  _folder->mkdir("T2-T0");
  _folder->cd("T2-T0");
  for(std::map<int,TH1F*>::iterator it=_T2mT0Ch.begin();it!=_T2mT0Ch.end();++it)
  { 
    it->second->Write();
  }
  _myfilestreamer<<_nbrRun<<std::endl;
  for(unsigned int i=0;i!=StreamerProba.size();++i)
  {
    for(std::map<int,std::pair<int,int>>::iterator it=StreamerProba[i].begin();it!=StreamerProba[i].end();++it)
    {
      std::cout<<red<<"Streamer probability for Chamber "<<it->first<<"  side " <<i<<"  "<<normal<<it->second.first*100.0/it->second.second<<std::endl;
      _myfilestreamer<<"Streamer probability for Chamber "<<it->first<<"  side " <<i<<"  "<<it->second.first*100.0/it->second.second<<std::endl;
    }
  }
  for(std::map<int,std::pair<int,int>>::iterator it=StreamerProbaBothSide.begin();it!=StreamerProbaBothSide.end();++it)
  {
      std::cout<<red<<"Streamer probability for Chamber "<<it->first<<"  BothSideGrouped "<<normal<<it->second.first*100.0/it->second.second<<std::endl;
      _myfilestreamer<<"Streamer probability for Chamber "<<it->first<<"  BothSideGrouped "<<it->second.first*100.0/it->second.second<<std::endl;
  }
  _myfilestreamer<<std::endl<<std::endl;
  _myfilestreamer.close();
  std::cout<<green<<_Selected<<" noise event and "<<_NotSelected<<" not selected"<<normal<<std::endl;
}

void ReadoutProcessor::fillTriggerBCIDInfo(TdcChannelBuffer &tdcBuf)
{
  _maxBCID=0;
  _BCIDwithTrigger.clear();
  _BCIDwithTriggerPerMezzanine.clear();
  _BCIDwithTriggerPerChamber.clear();
  for (TdcChannel* it=tdcBuf.begin(); it != tdcBuf.end(); ++it)
    {
      uint16_t bcid=it->bcid();
      if (_maxBCID<bcid) _maxBCID=bcid;
      if (it->channel()==triggerChannel) 
	{ 
	  _BCIDwithTrigger.insert(std::pair<uint16_t,double>(bcid,it->tdcTime()));  
	  _BCIDwithTriggerPerMezzanine[it->mezzanine()].push_back(bcid);
	  _BCIDwithTriggerPerChamber[IPtoChamber[it->mezzanine()]].push_back(*it);
	}
    }
  _maxBCID_histo->Fill(_maxBCID);
  _maxBCID_histozoom->Fill(_maxBCID);
  _triggerPerReadout->Fill(_BCIDwithTrigger.size());
  for (std::map<int,std::vector<uint16_t>>::iterator it =_BCIDwithTriggerPerMezzanine.begin();it!=_BCIDwithTriggerPerMezzanine.end();++it)
    _triggerPerReadoutPerMezzanine->Fill(it->second.size(),it->first);
}

void ReadoutProcessor::removeDataForChamberWithMoreThanOneTrigger(TdcChannelBuffer &tdcBuf)
{
  for(std::map<int,std::vector<TdcChannel>>::iterator it=_BCIDwithTriggerPerChamber.begin();it!=_BCIDwithTriggerPerChamber.end();++it)
    {
      std::pair<std::multimap<int,int>::iterator, std::multimap<int,int>::iterator> ret;
      ret=ChambertoIP.equal_range(it->first);
      if(it->second.size()>2)
	{
	  TdcChannel* iter=tdcBuf.end();
	  for(unsigned int i=0;i!=it->second.size();++i)
	    {
	      for(std::multimap<int,int>::iterator itt=ret.first; itt!=ret.second; ++itt)
		{
		  TdcChannel* rem=std::remove_if(tdcBuf.begin(), iter, TdcMezzaninePredicate(itt->second) );
		  tdcBuf.setEnd(rem);
		  iter=rem;
		}
	    }
	}
      else
	{
	  if(fabs(it->second[0].bcid()-it->second[1].bcid())<2&&(it->second[0].mezzanine()!=it->second[1].mezzanine()))continue;
	  TdcChannel* iter=tdcBuf.end();
	  for(std::multimap<int,int>::iterator itt=ret.first; itt!=ret.second; ++itt)
	    {
	      TdcChannel* rem=std::remove_if(tdcBuf.begin(), iter, TdcMezzaninePredicate(itt->second));
	      tdcBuf.setEnd(rem);
	      iter=rem;
	    }
	}
    }
}

void ReadoutProcessor::removeDataForMezzanineWithMoreThanOneTrigger(TdcChannelBuffer &tdcBuf)
{
  for (std::map<int,std::vector<uint16_t>>::iterator it =_BCIDwithTriggerPerMezzanine.begin();it!=_BCIDwithTriggerPerMezzanine.end();++it)
    if (it->second.size()>1)
      {
	TdcChannel* rem=std::remove_if(tdcBuf.begin(), tdcBuf.end(), TdcMezzaninePredicate(it->first) );
	tdcBuf.setEnd(rem);
      }
}

void ReadoutProcessor::processReadout(TdcChannelBuffer &tdcBuf,bool firstTime)
{
  for(static std::map<int,std::map<int,std::pair<int,double>>>::iterator it=_MinTimeFromTriggerInEvent.begin();it!=_MinTimeFromTriggerInEvent.end();++it)
  {
    (it->second).clear();
  }
  _MinTimeFromTriggerInEvent.clear();
  fillTriggerBCIDInfo(tdcBuf);
  //eventually here put a filter on the  BCIDwithTrigger set (like remove first ones, last ones, close ones)
  removeDataForChamberWithMoreThanOneTrigger(tdcBuf);
#ifdef LAURENT_STYLE
  removeDataForMezzanineWithMoreThanOneTrigger(tdcBuf);
#endif
  

  TdcChannel* eventStart=tdcBuf.begin();
  TdcChannel* eventEnd=nullptr;
  //std::cout << "Nombre de triggers = " << BCIDwithTrigger.size() << std::endl;
  for (std::set<std::pair<uint16_t,double>>::iterator it=_BCIDwithTrigger.begin(); it !=_BCIDwithTrigger.end(); ++it)
  {
    eventEnd=std::partition(eventStart,tdcBuf.end(),TdcChannelBcidpredicate((*it).first,(*it).second,-6,-3));
    processTrigger(eventStart,eventEnd,firstTime);
    eventStart=eventEnd;
  }
  if(firstTime==false)
  {
    if(_BCIDwithTrigger.size()==0&&tdcBuf.isNoise()==true)
    {
      _Selected++;
      processNoise(eventStart,tdcBuf.end());
    }
    else if (_BCIDwithTrigger.size()==0&&tdcBuf.isNoise()==false) 
    {
      _NotSelected++;
    }
  }
}


void ReadoutProcessor::processTrigger(TdcChannel* begin,TdcChannel* end,bool firstTime)
{ 
  TdcChannel* mezzStart=begin;
  TdcChannel* mezzEnd=nullptr;
  if(firstTime==false)
  {
    _tdc_counters.NewEvent();
    _chamberEfficiency.startEvent();
  }
  for(unsigned int i=0;i!=3;++i)_ugly[i].clear(); //reset _ugly array for clusterization
  _mul.clear();
  _mulchamber.clear();
  for(std::map<int,int>::iterator it=IPtoChamber.begin();it!=IPtoChamber.end();++it)
  {
    mezzEnd=std::partition(mezzStart,end,TdcMezzaninePredicate(it->first));
	  if(firstTime==true) processMezzanineFirst(mezzStart,mezzEnd);
	  else processMezzanine(mezzStart,mezzEnd);
	  mezzStart=mezzEnd;
  }
  doClusterize();
  fillHitMultiplicity();
  //if (nullptr != mezzEnd && end != mezzEnd) std::cout << "WARNING WARNING mess here" << std::endl;
  if(firstTime==false)
  {
    _counters.newSet();
    _chamberEfficiency.endEvent();
  }
}

void ReadoutProcessor::processNoise(TdcChannel* begin,TdcChannel* end)
{
  uint16_t _maxBCIDNoise=0;
  //uint16_t _minBCIDNoise=std::numeric_limits<uint16_t>::max();
  for(unsigned int i=0;i!=3;++i)_ugly[i].clear();
  std::map<int,int>noisehits;
  _data.Reset();
  for (TdcChannel* it=begin; it != end; ++it)
  {
    if(_maxBCIDNoise<it->bcid())_maxBCIDNoise=it->bcid();
    if(_minBCIDNoise>it->bcid())_minBCIDNoise=it->bcid();
    _ugly[it->side()].push_back(it);
    _ugly[2].push_back(it);
    _data.Push_back(it->side(),it->strip(),it->mezzanine(),it->tdcTime());
    noisehits[it->mezzanine()]++;
    noisehits[IPtoChamber[it->mezzanine()]]++;
  }
  double denominator=((_maxBCIDNoise/*-_minBCIDNoise*/)*2e-7*area);
  for(std::map<int,int>::iterator it=noisehits.begin();it!=noisehits.end();++it)
  {
    _noisehitspersecond->Fill(it->first,it->second*1.0/denominator);
  }
    for(unsigned int i=0;i!=3;++i)
  {
    RawHit_standard_merge_predicate Side(triggerChannel);
    Side.setNeighbourTimeDistance(NeighbourTimeDistance);
    Side.setNeighbourStripDistance(1);
    Side.setSide(i);
    std::vector<std::vector<TdcChannel*>::iterator> clusters;
    clusterize(_ugly[i].begin(),_ugly[i].end(),clusters,Side);
    std::map<int,int> NbrCluster;
    for(unsigned int j=0;j!=clusters.size()-1;++j)
    {
      NbrCluster[clusters[j][0]->chamber()]++;
      unsigned int clustersize=0;
      for(std::vector<TdcChannel*>::iterator itt=clusters[j];itt!=clusters[j+1];++itt)
      {
        clustersize++;
      }
      if(i==0)_MultiClusterNoiseSide0[clusters[j][0]->chamber()]->Fill(clustersize);
      else if(i==1)_MultiClusterNoiseSide1[clusters[j][0]->chamber()]->Fill(clustersize);
      else if(i==2)_MultiClusterNoiseBothSide[clusters[j][0]->chamber()]->Fill(clustersize);
    }
    for(std::map<int,int>::iterator it=NbrCluster.begin();it!=NbrCluster.end();++it)
    {
      if(i==0)_NbrClusterNoiseSide0[it->first]->Fill(it->second*1.0/denominator);
      else if(i==1)_NbrClusterNoiseSide1[it->first]->Fill(it->second*1.0/denominator);
      else if(i==2)_NbrClusterNoiseBothSide[it->first]->Fill(it->second*1.0/denominator);
    }
  }
  _data.OneNoise();
  _noiseTree->Fill();
}

bool isTrigger(TdcChannel& c) {return c.channel()==triggerChannel;}


void ReadoutProcessor::processMezzanineFirst(TdcChannel* begin,TdcChannel* end)
{
  int trigCount=std::count_if(begin,end,isTrigger);
  if (trigCount != 1) return;
  TdcChannel trigger(*(std::find_if(begin,end,isTrigger)));
  unsigned int valeur[2]={(unsigned int)trigger.chamber(),(unsigned int)trigger.mezzanine()};
  for (TdcChannel* it=begin; it !=end; ++it)
	{
	  if(it->channel()==triggerChannel) continue;
	  it->settdcTrigger(trigger.tdcTime());
	  if (it->side()==0) 
	  {
	    _t1mt0global->Fill(it->getTimeFromTrigger());
	    _t1mt0[it->chamber()]->Fill(it->getTimeFromTrigger());
	  }
	  else
	  {
	    _t2mt0global->Fill(it->getTimeFromTrigger());
	    _t2mt0[it->chamber()]->Fill(it->getTimeFromTrigger());
	  }
	  _tmt0global->Fill(it->getTimeFromTrigger());
	  _tmt0[it->chamber()]->Fill(it->getTimeFromTrigger());
	}
}
	
void ReadoutProcessor::finishFirst()
{
  for(std::map<int,TH1F*>::iterator oo=_tmt0.begin();oo!=_tmt0.end();++oo)
	{
    std::set<double>a=findPeaks(oo->second);
    std::set<double>::iterator yy=a.begin();
    double b=*(yy);
    yy++;
    double c=*(yy);
    TF1 *gauss = new TF1("gauss", "gaus");
    std::cout<<yellow<<b<<"  "<<c<<normal<<std::endl;
    gauss->SetParameter(1,b);
    gauss->SetParameter(2,(c+b)/2.0);
    std::cout<<blue<<(c-b)/2.0+b<<"  "<<(b-c)/2.0+b<<normal<<std::endl;
    oo->second->Fit("gauss","Q","",(b-c)/2.0+b,(c-b)/2.0+b);
    TF1 *fit1 = (TF1*)oo->second->GetFunction("gaus");
    if(fit1!=nullptr)
    {
      fit1->SetLineColor(kRed);
      fit1->Draw("same");
    }
    means[oo->first]=gauss->GetParameter(1);
    sigmas[oo->first]=gauss->GetParameter(2);
    _windowslow=gauss->GetParameter(1)-NbrOfSigmas*gauss->GetParameter(2);
    _windowshigh=gauss->GetParameter(1)+NbrOfSigmas*gauss->GetParameter(2);
    std::cout<<blue<<"Parameter chamber"<<oo->first<<" : Mean:"<<gauss->GetParameter(1)<<" Sigma:"<<gauss->GetParameter(2)<<normal<<std::endl;
    std::cout<<yellow<<"Time Windows for run "<<_nbrRun<<" {"<<_windowslow<<";"<<_windowshigh<<"}"<<normal<<std::endl;
    delete gauss;
    //delete fit1;
  }
  _tmt0global->Write();
  _t1mt0global->Write();
  _t2mt0global->Write();
  _myfilepeaks<<_nbrRun<<std::endl;
  for(std::map<int,TH1F*>::iterator it=_tmt0.begin();it!=_tmt0.end();++it)
  {
    _myfilepeaks<<"T-T0 Chamber "<<it->first<<" : ";
    std::set<double> a = findPeaks(it->second);
    for(std::set<double>::iterator itt=a.begin();itt!=a.end();++itt)
    {
      _myfilepeaks<<(*itt)<<" ";
    }
    _myfilepeaks<<std::endl;
    it->second->Write();
    delete it->second;
  }
   for(std::map<int,TH1F*>::iterator it=_t1mt0.begin();it!=_t1mt0.end();++it)
  {
    _myfilepeaks<<"T1-T0 Chamber "<<it->first<<" : ";
    std::set<double> a = findPeaks(it->second);
    for(std::set<double>::iterator itt=a.begin();itt!=a.end();++itt)
    {
      _myfilepeaks<<(*itt)<<" ";
    }
    _myfilepeaks<<std::endl;
    it->second->Write();
    delete it->second;
  }
   for(std::map<int,TH1F*>::iterator it=_t2mt0.begin();it!=_t2mt0.end();++it)
  {
    _myfilepeaks<<"T2-T0 Chamber "<<it->first<<" : ";
    std::set<double> a = findPeaks(it->second);
    for(std::set<double>::iterator itt=a.begin();itt!=a.end();++itt)
    {
      _myfilepeaks<<(*itt)<<" ";
    }
    _myfilepeaks<<std::endl;
    it->second->Write();
    delete it->second;
  }
  _myfile<<std::endl;
  _myfile.close();
}

void ReadoutProcessor::processMezzanine(TdcChannel* begin,TdcChannel* end)
{
  _OnlyOne.clear();
  _data.Reset();
  int trigCount=std::count_if(begin,end,isTrigger);
  if (trigCount != 1) return;
  TdcChannel trigger(*(std::find_if(begin,end,isTrigger)));
  unsigned int valeur[2]={(unsigned int)trigger.chamber(),(unsigned int)trigger.mezzanine()};
  _tdc_counters.YouAreConcernedByATrigger(trigger.bcid(),valeur);
  _chamberEfficiency.setTriggerSeen((unsigned int)trigger.mezzanine());
  for (TdcChannel* it=begin; it !=end; ++it)
	{
	  if(it->channel()==triggerChannel) continue;
	  it->settdcTrigger(trigger.tdcTime());
	  _triggerTime->Fill(it->getTimeFromTrigger());
    if (it->side()==0) _hitTimePair[it->mezzanine()]->Fill(it->getTimeFromTrigger());
    else _hitTimeImpair[it->mezzanine()]->Fill(it->getTimeFromTrigger());
    if (_T1mT0Ch.find(it->strip())==_T1mT0Ch.end())
	  {
	    _T1mT0Ch[it->strip()]=new TH1F(("T1-T0_"+std::to_string(it->strip())).c_str(),("T1-T0_"+std::to_string(it->strip())).c_str(),20000,-2000,0);
	    _T2mT0Ch[it->strip()]=new TH1F(("T2-T0_"+std::to_string(it->strip())).c_str(),("T2-T0_"+std::to_string(it->strip())).c_str(),20000,-2000,0);
	    _TimeWithRespectToFirstOneCh2d0[it->strip()]=new TH2F(("Timerespecttofirstone2d0_for_strip_"+std::to_string(it->strip())).c_str(),("Timerespecttofirstone2d0_fot_strip_"+std::to_string(it->strip())).c_str(),20,-10,10,1200,-60,60);
	    _TimeWithRespectToFirstOneCh2d1[it->strip()]=new TH2F(("Timerespecttofirstone2d1_for_strip_"+std::to_string(it->strip())).c_str(),("Timerespecttofirstone2d1_fot_strip_"+std::to_string(it->strip())).c_str(),20,-10,10,1200,-60,60);
	    _TimeWithRespectToFirst[it->strip()]=new TH1F(("TimeWithRespectToFirst_"+std::to_string(it->strip())).c_str(),("TimeWithRespectToFirst_"+std::to_string(it->strip())).c_str(),20000,-5000,5000);
      _TimeWithRespectToFirstSameStrip0[it->strip()]=new TH1F(("TimeWithRespectToFirstSameStrip0_"+std::to_string(it->strip())).c_str(),("TimeWithRespectToFirstSameStrip0_"+std::to_string(it->strip())).c_str(),2500,0,250);
       _TimeWithRespectToFirstSameStrip1[it->strip()]=new TH1F(("TimeWithRespectToFirstSameStrip1_"+std::to_string(it->strip())).c_str(),("TimeWithRespectToFirstSameStrip1_"+std::to_string(it->strip())).c_str(),2500,0,250);
       _DistributionHitCloseTrigger[it->strip()]=new TH1F(("Closesttotrigger"+std::to_string(it->strip())).c_str(),("Closesttotrigger"+std::to_string(it->strip())).c_str(),2000,-2000,0);
	  }
	  if (it->side()==0) 
	  {
	    _T1mT0Ch[it->strip()]->Fill(it->getTimeFromTrigger());
	  }
	  else
	  {
	    _T2mT0Ch[it->strip()]->Fill(it->getTimeFromTrigger());
	  }
	  _data.Push_back(it->side(),it->strip(),it->mezzanine(),it->tdcTime(),trigger.tdcTime());
	}
  int to_add=0;
  if (int(end-begin)>1) //at least one hit more than the trigger
  {
    TdcChannel* endTrigWindow=std::remove_if(begin,end,TdcOutofTriggerTimePredicate(trigger.tdcTime(),_windowslow,_windowshigh));
    for(std::map<int,std::map<int,std::pair<int,double>>>::iterator tt=_MinTimeFromTriggerInEvent.begin();tt!=_MinTimeFromTriggerInEvent.end();++tt)
    {
      for(std::map<int,std::pair<int,double>>::iterator op=tt->second.begin();op!=tt->second.end();++op)
      {
        _DistributionHitCloseTrigger[(op->second).first]->Fill((op->second).second);
      }
    }
    if (endTrigWindow!=begin) {to_add=1; _tdc_counters.YouHaveAHit(trigger.bcid(),valeur);}
    for (TdcChannel* it=begin; it !=endTrigWindow; ++it)
	  {
	    if (it->channel()==triggerChannel) continue;
	    _chamberEfficiency.setHitSeen((unsigned int)it->mezzanine());
      _mul[it->side()][it->chamber()]++;
      _mulchamber[it->chamber()]++;
      if(_OnlyOne.find(it->side()*10000+it->strip())==_OnlyOne.end()) _OnlyOne[it->side()*10000+it->strip()]=it;
      else if(_OnlyOne[it->side()*10000+it->strip()]->getTimeFromTrigger()>it->getTimeFromTrigger()) _OnlyOne[it->side()*10000+it->strip()]=it;
	    _ugly[it->side()].push_back(it);
	    _ugly[2].push_back(it);
	  }
    for(TdcChannel* it=begin; it != endTrigWindow; ++it)
	  {
	    if(it->channel()==triggerChannel) continue;
	    _TimeWithRespectToFirst[it->strip()]->Fill(_MinTimeFromTriggerInEvent[it->mezzanine()][it->strip()%100].second-it->getTimeFromTrigger());
	    TdcChannel* beginpp=it;
	    beginpp++;
	    for(TdcChannel* itt=begin; itt != endTrigWindow; ++itt)
	    {
	      if(itt->channel()==triggerChannel) continue;
	      if(it->chamber()==itt->chamber())
	      {
	         if(it->side()==0&&itt->side()==0)_TimeWithRespectToFirstOneCh2d0[it->strip()]->Fill((itt->strip()%100)-(it->strip()%100),itt->getTimeFromTrigger()-it->getTimeFromTrigger());
	         else if(it->side()==1&&itt->side()==1)_TimeWithRespectToFirstOneCh2d1[it->strip()]->Fill((itt->strip()%100)-(it->strip()%100),itt->getTimeFromTrigger()-it->getTimeFromTrigger());
	        if(itt>beginpp)
	        {
	          _Correlation[it->chamber()]->Fill(it->strip()%100,itt->strip()%100);
	        }
	      }
	      if(it->strip()==itt->strip())
		    {
		      if(it->side()==0&&itt->side()==0)_TimeWithRespectToFirstSameStrip0[it->strip()]->Fill(_MinTimeFromTriggerInEvent[it->mezzanine()][it->strip()%100].second-itt->getTimeFromTrigger());
		      else if(it->side()==1&&itt->side()==1)_TimeWithRespectToFirstSameStrip1[it->strip()]->Fill(_MinTimeFromTriggerInEvent[it->mezzanine()][it->strip()%100].second-itt->getTimeFromTrigger());
		      if(_T1mT2Ch.find(it->strip())==_T1mT2Ch.end())
		      {
		        _T1mT2Ch[it->strip()]=new TH1F(("T1-T2_"+std::to_string(it->strip())).c_str(),("T1-T2_"+std::to_string(it->strip())).c_str(),10000,-500,500);
		        _T1mT2ChOneHit[it->strip()]=new TH1F(("T1-T2_onehit_"+std::to_string(it->strip())).c_str(),("T1-T2_onehit_"+std::to_string(it->strip())).c_str(),200,-20,20);
		      }
		      double diff=it->getTimeFromTrigger()-itt->getTimeFromTrigger();
		      double sum=std::fabs(it->getTimeFromTrigger())+std::fabs(itt->getTimeFromTrigger())+means[it->chamber()]+means[itt->chamber()];
		      if((it->side()+1)==itt->side())
		      {
		        _T1mT2[it->chamber()]->Fill(it->strip()%100,diff);
		        _Position[it->chamber()]->Fill(it->strip()%100,(diff*vitesse+longueur)/2);
		        _Longueur[it->chamber()]->Fill(it->strip()%100,sum*vitesse);
		        _T1mT2Ch[it->strip()]->Fill(diff);
		        _T1mT2Chamber[it->chamber()]->Fill(diff);
		      } 
		      else if (it->side()==(itt->side()+1))
		      {
		        _T1mT2[it->chamber()]->Fill(it->strip()%100,-diff);
		        _T1mT2Ch[it->strip()]->Fill(-diff);
		        _T1mT2Chamber[it->strip()/100]->Fill(-diff);
		        _Position[it->chamber()]->Fill(it->strip()%100,float((-diff*vitesse+longueur)/2));
		        _Longueur[it->chamber()]->Fill(it->strip()%100,sum*vitesse);
		      }
		    }
	    }
	  }
  }
  for(std::map<int,TdcChannel*>::iterator tt=_OnlyOne.begin();tt!=_OnlyOne.end();++tt)
  {
    for(std::map<int,TdcChannel*>::iterator ttt=tt;ttt!=_OnlyOne.end();++ttt)
    {
      double diff=tt->second->getTimeFromTrigger()-ttt->second->getTimeFromTrigger();
      double sum=std::fabs(tt->second->getTimeFromTrigger())+std::fabs(ttt->second->getTimeFromTrigger())+means[tt->second->chamber()]+means[ttt->second->chamber()];
      if(tt->second->strip()==ttt->second->strip()&&tt->second->side()+1==ttt->second->side())
      {
        /*if(sum*vitesse>(longueur-incertitude*vitesse)&&sum*vitesse<(longueur+incertitude*vitesse))*/_T1mT2ChOneHit[tt->second->strip()]->Fill(diff);
      }
      else if (tt->second->strip()==ttt->second->strip()&&tt->second->side()==ttt->second->side()+1)
      {
        /*if(sum*vitesse>(longueur-incertitude*vitesse)&&sum*vitesse<(longueur+incertitude*vitesse))*/_T1mT2ChOneHit[tt->second->strip()]->Fill(-diff);
      }
    }
  }
  _data.OneEvent();
  _dataTree->Fill();
  ///
  _counters.add(to_add,valeur);
}

void ReadoutProcessor::fillHitMultiplicity()
{
  for(std::map<int,std::map<int,int>>::iterator it=_mul.begin();it!=_mul.end();++it)
  {
    for(std::map<int,int>::iterator itt=it->second.begin();itt!=it->second.end();++itt)
    {
      if(itt->second>=NbrStreamer)
      {
        StreamerProba[it->first][itt->first].first++;
      }
      StreamerProba[it->first][itt->first].second++;
      if(it->first==0)_MultiplicitySide0[itt->first]->Fill(itt->second);
      if(it->first==1)_MultiplicitySide1[itt->first]->Fill(itt->second);
    }
  }
  for(std::map<int,int>::iterator it=_mulchamber.begin();it!=_mulchamber.end();++it)
  {
    if(it->second>=2*NbrStreamer)
    {
      StreamerProbaBothSide[it->first].first++;
    }
    StreamerProbaBothSide[it->first].second++;
    _MultiplicityBothSide[it->first]->Fill(it->second);
  }
}

double TdcChannelClusterWrapper::meanStrip()
{
  int N=_cl.size();
  if (0==N) return -999;
  double S=0;
  for (auto it=_cl.begin(); it!=_cl.end(); ++it) S+=(**it)->strip();
  return S/N;
}

double TdcChannelClusterWrapper::meanTime()
{
  int N=_cl.size();
  if (0==N) return -999;
  double S=0;
  for (auto it=_cl.begin(); it!=_cl.end(); ++it) S+=(**it)->getTimeFromTrigger();
  return S/N;
}


void ReadoutProcessor::fillClusterTSideConnectedTOtherSide(std::map<unsigned int, std::vector<unsigned int> >& result, std::vector<Cluster<TdcChannel*> >& side, std::vector<Cluster<TdcChannel*> >& otherSide)
{
  for (unsigned int j=0; j<side.size(); ++j)
    {
      TdcChannelClusterWrapper sideclus(side[j]);
      double meanStrip=sideclus.meanStrip();
      for (unsigned k=0; k<otherSide.size(); ++k)
	{
	  TdcChannelClusterWrapper otherSideclus(otherSide[k]);
	  if (abs(meanStrip-otherSideclus.meanStrip()) <=1 ) result[j].push_back(k);
	}
    }
}

void ReadoutProcessor::reportClusterTSideConnectedTOtherSide(std::map<unsigned int, std::vector<unsigned int> >& connexionMap, std::vector<Cluster<TdcChannel*> >& side, std::string sideName, std::vector<Cluster<TdcChannel*> >& otherSide, std::string otherSideName)
{
  for (std::map<unsigned int, std::vector<unsigned int> >::iterator it=connexionMap.begin(); it != connexionMap.end(); ++it)
    if (it->second.size() >1)
      {
	TdcChannelClusterWrapper sideClus(side[it->first]);
	std::cout << "WHAT SHOULD I DO ? cluster on " << sideName <<" side (strip=" << sideClus.meanStrip() << " time=" << sideClus.meanTime()
		  << ") is strip compatible with " << it->second.size() << " clusters on the "<< otherSideName << " side :" << std::endl;
	for (std::vector<unsigned int>::iterator itb=it->second.begin(); itb!=it->second.end(); ++itb)
	  {
	    TdcChannelClusterWrapper otherSideClus(otherSide[*itb]);
	    std::cout << "       ---- "<< otherSideName <<" cluster (strip=" << otherSideClus.meanStrip() << " time=" << otherSideClus.meanTime() << ")"<< std::endl;
	  }
      }
}


void ReadoutProcessor::doClusterize()
{
  std::vector<Cluster<TdcChannel*> > clustersT1side;
  std::vector<Cluster<TdcChannel*> > clustersT2side;
  for(unsigned int i=0;i!=3;++i)
    {
    RawHit_standard_merge_predicate Side(triggerChannel);
    Side.setNeighbourTimeDistance(NeighbourTimeDistance);
    Side.setNeighbourStripDistance(1);
    Side.setSide(i);
    std::vector<std::vector<TdcChannel*>::iterator> clusters;
    clusterize(_ugly[i].begin(),_ugly[i].end(),clusters,Side);
    std::map<int,int> NbrCluster;
    for(unsigned int j=0;j!=clusters.size()-1;++j)
    {
      NbrCluster[clusters[j][0]->chamber()]++;
      unsigned int clustersize=0;
      for(std::vector<TdcChannel*>::iterator itt=clusters[j];itt!=clusters[j+1];++itt)
      {
        clustersize++;
      }
      if(i==0)_MultiClusterSide0[clusters[j][0]->chamber()]->Fill(clustersize);
      else if(i==1)_MultiClusterSide1[clusters[j][0]->chamber()]->Fill(clustersize);
      else if(i==2)_MultiClusterBothSide[clusters[j][0]->chamber()]->Fill(clustersize);
    }
    for(std::map<int,int>::iterator it=NbrCluster.begin();it!=NbrCluster.end();++it)
    {
      if(i==0)_NbrClusterSide0[it->first]->Fill(it->second);
      else if(i==1)_NbrClusterSide1[it->first]->Fill(it->second);
      else if(i==2)_NbrClusterBothSide[it->first]->Fill(it->second);
    }
    if (i==0) Convert(clusters,clustersT1side);
    if (i==1) Convert(clusters,clustersT2side);
    if (i==2) doTimeAnalyzeClusters(clusters);
  }
  std::map<unsigned int, std::vector<unsigned int> > T1indexStripConnectWithT2;
  std::map<unsigned int, std::vector<unsigned int> > T2indexStripConnectWithT1;
  fillClusterTSideConnectedTOtherSide(T1indexStripConnectWithT2,clustersT1side,clustersT2side);
  fillClusterTSideConnectedTOtherSide(T2indexStripConnectWithT1,clustersT2side,clustersT1side);
  reportClusterTSideConnectedTOtherSide(T1indexStripConnectWithT2,clustersT1side,"T1",clustersT2side,"T2");
}

void ReadoutProcessor::doTimeAnalyzeClusters(std::vector<std::vector<TdcChannel*>::iterator> &clusterBounds)
{
  for(unsigned int j=0;j!=clusterBounds.size()-1;++j)
    {
      _clusterTimeAnalysisCut->Fill(0.5);
      unsigned int clusterSize=std::distance(clusterBounds[j],clusterBounds[j+1]);
      if (clusterSize<2) continue;
      _clusterTimeAnalysisCut->Fill(1.5);
      typedef int stripNumber;
      typedef int stripSide;
      std::map<stripNumber,std::map<stripSide,std::vector<double> > > timeInfo;
      bool timeInfoIsDirty=false;
      for (std::vector<TdcChannel*>::iterator ithit=clusterBounds[j]; ithit!=clusterBounds[j+1]; ++ithit)
	{
	  timeInfo[(**ithit).strip()][(**ithit).side()].push_back((**ithit).getTimeFromTrigger());
	  if (timeInfo[(**ithit).strip()][(**ithit).side()].size()>1) {timeInfoIsDirty=true; break;}
	  //std::cout << " " << **ithit;
	}
      if (timeInfoIsDirty) continue;
      _clusterTimeAnalysisCut->Fill(2.5);
      std::pair<int,double> T1mT0accumulate(0,0.0),T2mT0accumulate(0,0.0),T1mT2accumulate(0,0.0);
      for (auto itStrip=timeInfo.begin(); itStrip!=timeInfo.end(); ++itStrip)
	{
	  if (itStrip->second.size()==2)
	    {++T1mT2accumulate.first; T1mT2accumulate.second+=(itStrip->second[0][0]-itStrip->second[1][0]);}
	  for (auto itSide=itStrip->second.begin(); itSide!=itStrip->second.end(); ++itSide)
	    {
	      if (itSide->first==0) {++T1mT0accumulate.first; T1mT0accumulate.second+=itSide->second[0];}
	      if (itSide->first==1) {++T2mT0accumulate.first; T2mT0accumulate.second+=itSide->second[0];}
	    }
	}
      if (T1mT0accumulate.first==0 or T2mT0accumulate.first==0) continue;
      _clusterTimeAnalysisCut->Fill(3.5);
      _NumberOfStripsForT1mT0inCluster[(**(clusterBounds[j])).chamber()]->Fill(T1mT0accumulate.first);
      _NumberOfStripsForT2mT0inCluster[(**(clusterBounds[j])).chamber()]->Fill(T2mT0accumulate.first);
      _MeanT1mimusMeanT2inCluster[(**(clusterBounds[j])).chamber()]->Fill(T1mT0accumulate.second/T1mT0accumulate.first-T2mT0accumulate.second/T2mT0accumulate.first);
      if (T1mT2accumulate.first==0) continue;
      _clusterTimeAnalysisCut->Fill(4.5);
      _NumberOfStripsForT1mT2inCluster[(**(clusterBounds[j])).chamber()]->Fill(T1mT2accumulate.first);
      _MeanT1mimusT2inCluster[(**(clusterBounds[j])).chamber()]->Fill(T1mT2accumulate.second/T1mT2accumulate.first);
      if (T1mT2accumulate.first>2) _MeanT1mimusT2inCluster3StripMin[(**(clusterBounds[j])).chamber()]->Fill(T1mT2accumulate.second/T1mT2accumulate.first);
    }
}
