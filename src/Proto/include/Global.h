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
static unsigned int triggerChannel=16;
static double vitesse=2.0/3*29.979245800; //2/3 de la vitesse de la lumiere
static double longueur=100;
static int numbereventtoprocess=0;
#endif
