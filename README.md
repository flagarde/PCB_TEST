# PCB_TEST
program to convert .dat to ROOTFile similar to the GIF++ conventions

1) cd PCB_TEST
2) mkdir build
3) You can change some variable in PCB_TEST/Proto/include/Global.h
4) you can change the data format (MayData or NewData) by changing the line option(IS_MAY_DATA "May Data" ON) in PCB_TEST/src/Proto/CMakeLists.txt or by loading it like cmake -DIS_MAY_DATA=OFF/ON .. 
5) make install
6) The program is inside PCB_TEST/bin and is called Read (Multi-file corresponding to one run number will be analysed as one file)
