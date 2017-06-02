#ifndef GGlobal
#define GGlobal
#include <array>
#include <map>
static std::array<std::array<int,32>,2>TDCchannelToStrip
{{
  {15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0},
  {16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31}
}};
static std::map<int,int>IPtoChamber{{15,1},{14,1},{13,2},{12,2}};
static std::multimap<int,int>ChambertoIP{{1,15},{1,14},{2,13},{2,12}};
static unsigned int const triggerChannel=16;
static double const vitesse=2.0/3*29.979245800; //2/3 de la vitesse de la lumiere
static double const longueur=100;
static int const nbrTotalHitsMax=1000;
static int const  area=50*0.4*16;//50cmlong*0.4width*16strips
static int const NbrStreamer=7;
static std::map<std::pair<int,int>,std::pair<int,int>>Windows
{
{{0,735976},{-900,-861}},
{{735977,735985},{-623,-601}},
{{735986,736007},{-900,-861}},
{{736007,736027},{-920,-890}},
{{736028,736032},{-635,-610}},
{{736033,736033},{-730,-710}},
{{736034,736087},{-635,-610}},
{{736088,736088},{-690,-660}},
{{736089,736089},{-805,-770}},
{{736090,736094},{-635,-610}},
{{736095,736095},{-750,-680}},
{{736096,736096},{-635,-610}},
{{736097,736097},{-730,-680}},
{{736098,736098},{-635,-610}},
{{736099,736099},{-760,-680}},
{{736100,736100},{-635,-610}},
{{736101,736101},{-760,-680}},
{{736102,736102},{-635,-610}},
{{736103,736103},{-760,-680}},
{{736104,736104},{-635,-610}},
{{736105,736105},{-760,-680}},
{{736106,736106},{-635,-610}},
{{736107,736107},{-760,-680}},
{{736108,736185},{-635,-610}},
};
static uint64_t FourSecondsInClockTicks=20000000;
static uint64_t TenSecondsInClockTicks=50000000;
static std::map<int,std::pair<int,double>> _MinTimeFromTriggerInEvent;
#endif
