#ifndef GGlobal
#define GGlobal
#include <array>
#include <map>
#include <vector>
#include <string>

#ifndef MayData
static std::string dataType="Not May Data";
static std::array<std::array<int,32>,1>LEMO2STRIP
{
    {86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,
           71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86}
};

static std::array<std::array<int,32>,1>PR2LEMO
{
{11,12,10,13,9,14,8,7,6,5,4,3,2,1,0,31,
          30,29,28,27,26,25,24,23,15,22,16,21,17,20,18,19}
};

static std::array<std::array<int,32>,1>TDC2PR
{
{31,0,30,1,29,2,28,3,27,4,26,5,25,6,24,7,
          -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,}
};

static bool SupressEventWithMoreThanOneTriggerByChamber=false;
extern std::vector<std::vector<int>>TDCchannelToStrip;
extern std::map<int,int>IPtoChamber;
static std::multimap<int,int>ChambertoIP{{1,5}};
static unsigned int const detectorId=120;
#else
static std::string dataType="May Data";
extern std::map<int,int>IPtoChamber;
static bool SupressEventWithMoreThanOneTriggerByChamber=false;
static std::array<std::array<int,32>,2>TDCchannelToStrip
{{
  {15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0},
  {16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31}
}};
static std::multimap<int,int>ChambertoIP{{1,15},{1,14},{2,13},{2,12}};
static unsigned int const detectorId=110;
#endif

//////////////Window Selection [Ttrigger+NumberOfClockTickBefore,Ttrigger-NumberOfClockTickAfter]
///// I'm adding +1 for clock problem sometimes
static int NumberOfClockTickBefore=-2;
static int NumberOfClockTickAfter=+1;
/////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned int const triggerChannel=16;
static double const vitesse=2.0/3*29.979245800; //2/3 de la vitesse de la lumiere
static double const longueur=137;
static int const nbrTotalHitsMax=1000;
static int const  area=50*0.4*16;//50cmlong*0.4width*16strips
static int const NbrStreamer=7;
static uint64_t FourSecondsInClockTicks=20000000;
static uint64_t TenSecondsInClockTicks=50000000;
static std::map<int,std::map<int,std::pair<int,double>>> _MinTimeFromTriggerInEvent;
static std::map<int,double>means;
static std::map<int,double>sigmas;
static int NeighbourTimeDistance=2; //ns
static int incertitude=4; //ns
static double NbrOfSigmas=1.5;
#endif
