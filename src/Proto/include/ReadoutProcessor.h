#ifndef ReadoutProcessor_h
#define ReadoutProcessor_h 
#include "TdcChannelBuffer.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "GG_counter.h"
#include "TdcChamberEfficiency.hh"
#include <cstdint>
#include "TBranch.h"
#include "TTree.h"
#include "RAWData.h"
#include "TProfile.h"
#include "TFile.h"
#include <fstream>

#include "Clustering.h"
//helper class for time clustering studies
class TdcChannelClusterWrapper 
{
 public:
 TdcChannelClusterWrapper(Cluster<TdcChannel*>& cl) : _cl(cl) {}
  double meanStrip();
  double meanTime();
  Cluster<TdcChannel*>& cluster() {return _cl;}
 private:
  Cluster<TdcChannel*>& _cl;
};

class ReadoutProcessor
{
public:
  ReadoutProcessor(int NbrEventToProcess,TFile* fol,std::string& nbr):_numbereventtoprocess(NbrEventToProcess),_folder(fol),_nbrRun(nbr){}
  void init();
  int readstream(int32_t _fdIn,bool firstTime);
  void processReadout(TdcChannelBuffer &tdcBuf,bool firstTime);
  void processTrigger(TdcChannel* begin,TdcChannel* end,bool firstTime);
  void processMezzanine(TdcChannel* begin,TdcChannel* end);
  void processMezzanineFirst(TdcChannel* begin,TdcChannel* end);
  void processNoise(TdcChannel* begin,TdcChannel* end);
  void finish();
  void finishFirst();
private:
  std::string _nbrRun{""};
  int _windowslow{0};
  int _windowshigh{0};
  uint16_t _maxBCID;
  uint16_t _minBCIDNoise;
  TH1F* _maxBCID_histo=nullptr;
  TH1F* _maxBCID_histozoom=nullptr;
  TH1F* _triggerPerReadout=nullptr;
  TH2F* _triggerPerReadoutPerMezzanine=nullptr;
  TH1F* _triggerTime=nullptr;
  std::map<unsigned int,TH1F*> _hitTimePair;
  std::map<unsigned int,TH1F*> _hitTimeImpair;
  TH1F* _nDIFinReadout=nullptr;
  TH1F* _nReadoutperAbsBCID=nullptr;
  std::map<uint64_t,unsigned int> _AbsBCID_Readout_map;
  uint64_t _lastTriggerAbsBCID{0};
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
  TDC_ChamberCounters _tdc_counters;
  TdcMultiChamberEfficiency _chamberEfficiency;
  int32_t _numbereventtoprocess;
  int32_t _totalevent{0};
  TFile* _folder=nullptr;
  TH1F* _tmt0global{new TH1F("T-T0 global","T-T0 global",20000,-2000,0)};
  TH1F* _t1mt0global{new TH1F("T1-T0 global","T1-T0 global",20000,-2000,0)};
  TH1F* _t2mt0global{new TH1F("T2-T0 global","T2-T0 global",20000,-2000,0)};
  std::map<int,TH1F*> _tmt0;
  std::map<int,TH1F*> _t1mt0;
  std::map<int,TH1F*> _t2mt0;
  std::map<int,TH1F*> _MultiplicitySide0;
  std::map<int,TH1F*> _MultiplicitySide1;
  std::map<int,TH1F*> _MultiplicityBothSide;
  std::map<int,TH2F*> _T1mT2;
  std::map<int,TH2F*> _Correlation;
  std::map<int,TH1F*> _T1mT2Ch;
  std::map<int,TH1F*> _T1mT2ChOneHit;
  std::map<int,TH1F*> _T1mT0Ch;
  std::map<int,TH1F*> _T2mT0Ch;
  std::map<int,TH2F*> _TimeWithRespectToFirstOneCh2d0;
  std::map<int,TH2F*> _TimeWithRespectToFirstOneCh2d1;
  std::map<int,TH1F*> _TimeWithRespectToFirst;
  std::map<int,TH1F*> _DistributionHitCloseTrigger;
  std::map<int,TH1F*> _TimeWithRespectToFirstSameStrip0;
  std::map<int,TH1F*> _TimeWithRespectToFirstSameStrip1;
  std::map<int,TH1F*> _T1mT2Chamber;
  std::map<int,TH2F*> _Position;
  std::map<int,TH2F*> _Longueur;
  std::map<int,TH1F*> _NbrClusterSide0;
  std::map<int,TH1F*> _MultiClusterSide0;
  std::map<int,TH1F*> _NbrClusterSide1;
  std::map<int,TH1F*> _MultiClusterSide1;
  std::map<int,TH1F*> _NbrClusterBothSide;
  std::map<int,TH1F*> _MultiClusterBothSide;
  std::map<int,TH1F*> _NbrClusterNoiseSide0;
  std::map<int,TH1F*> _MultiClusterNoiseSide0;
  std::map<int,TH1F*> _NbrClusterNoiseSide1;
  std::map<int,TH1F*> _MultiClusterNoiseSide1;
  std::map<int,TH1F*> _NbrClusterNoiseBothSide;
  std::map<int,TH1F*> _MultiClusterNoiseBothSide;
  std::map<int,TH1F*> _NumberOfStripsForT1mT0inCluster;
  std::map<int,TH1F*> _NumberOfStripsForT2mT0inCluster;
  std::map<int,TH1F*> _NumberOfStripsForT1mT2inCluster;
  std::map<int,TH1F*> _MeanT1mimusMeanT2inCluster;
  std::map<int,TH1F*> _MeanT1mimusT2inCluster;
  std::map<int,TH1F*> _MeanT1mimusT2inCluster3StripMin;
  TH1F* _clusterTimeAnalysisCut;
  std::array<std::vector<TdcChannel*>,3> _ugly;
  std::map<int,std::map<int,int>> _mul;
  std::map<int,int> _mulchamber;

  
  std::map<int,TdcChannel*> _OnlyOne;
  std::array<std::map<int,std::pair<int,int>>,2> StreamerProba;
  std::map<int,std::pair<int,int>> StreamerProbaBothSide;
  std::ofstream _myfile;
  std::ofstream _myfilestreamer;
  std::ofstream _myfilepeaks;
  uint32_t _Selected{0};
  uint32_t _NotSelected{0};
  //processReadoutHelper
  std::set<std::pair<uint16_t,double>> _BCIDwithTrigger;
  std::map<int,std::vector<uint16_t> > _BCIDwithTriggerPerMezzanine;
  std::map<int,std::vector<TdcChannel>> _BCIDwithTriggerPerChamber;
  void fillTriggerBCIDInfo(TdcChannelBuffer &tdcBuf);
  void removeDataForChamberWithMoreThanOneTrigger(TdcChannelBuffer &tdcBuf);
  void removeDataForMezzanineWithMoreThanOneTrigger(TdcChannelBuffer &tdcBuf);
  void doClusterize();
  void doTimeAnalyzeClusters(std::vector<std::vector<TdcChannel*>::iterator> &clusterBounds);
  void fillHitMultiplicity();
  void fillClusterTSideConnectedTOtherSide(std::map<unsigned int, std::vector<unsigned int> >& result, std::vector<Cluster<TdcChannel*> >& side, std::vector<Cluster<TdcChannel*> >& otherSide);
  void reportClusterTSideConnectedTOtherSide(std::map<unsigned int, std::vector<unsigned int> >& connexionMap, std::vector<Cluster<TdcChannel*> >& side, std::string sideName, std::vector<Cluster<TdcChannel*> >& otherSide, std::string otherSideName);
  class ClusterSideHistos
  {
  public:
    ClusterSideHistos() {}
    ClusterSideHistos(std::string sideName) {book(sideName);}
    ~ClusterSideHistos();
    TH1F *_NClusters=nullptr;
    TH1F *_ClusterSize=nullptr;
    TGraph *_StripVsDT=nullptr;
    std::map<int,TGraph *> _DStripVsDTByStrip;
    std::map<int,TProfile *> _DTVsDStripByStrip;
    void book(std::string sideName);
    void fill(std::vector<Cluster<TdcChannel*> >& clustersVec);
    void write();
  private:
    void fillOneCluster(Cluster<TdcChannel*> &cluster);
    void addGraphPoint(TdcChannel* ref,TdcChannel* second);
    void addGraphPointByStrip(TdcChannel* first,TdcChannel* second);
    std::string _sideName;
  };
  ClusterSideHistos _T1SideClusterHistos;
  ClusterSideHistos _T2SideClusterHistos;
  class ClusterSideCorrelHistos
  {
  public:
    ClusterSideCorrelHistos() {}
    ClusterSideCorrelHistos(std::string refSideName, std::string otherSideName) {book(refSideName,otherSideName);}
    TH1F *_NAssociatedClusters=nullptr;
    TGraph *_StripVsDT;
    void book(std::string refSideName, std::string otherSideName);
    void fill(std::map<unsigned int, std::vector<unsigned int> >& connexionMap, std::vector<Cluster<TdcChannel*> >& refSide, std::vector<Cluster<TdcChannel*> >& otherSide);
    void write();
  private:
    void addGraphPoint(TdcChannelClusterWrapper& ref,TdcChannelClusterWrapper& second);
  };
  ClusterSideCorrelHistos _T1toT2ClusterSideCorrelHistos;
  ClusterSideCorrelHistos _T2toT1ClusterSideCorrelHistos;  
};
#endif 
