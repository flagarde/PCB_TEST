#ifndef H_TdcChannelBuffer
#define H_TdcChannelBuffer
#include "TdcChannels.hh"
#include <cstdint>
class TdcChannelBuffer
{
public:
  TdcChannelBuffer(){};
  TdcChannelBuffer(uint32_t max_size){ _ptr=new char[max_size*sizeof(TdcChannel)]; _end=_Tdcptr=(TdcChannel*) _ptr;}
  ~TdcChannelBuffer() { delete [] _ptr;}
  void addChannel(uint8_t* b) { new (_end) TdcChannel(b); ++_end;}
  TdcChannel& operator[](uint32_t i) {return _Tdcptr[i];}
  TdcChannel& last() {return  *(_end-1);}
  TdcChannel* begin() {return _Tdcptr;}
  TdcChannel* end() {return _end;}
  void clear() {_end=_Tdcptr;}
  uint32_t nTdcChannel() {return _end-_Tdcptr;}
  void setEnd(TdcChannel* end) {_end=end;}
  bool isNoise(){return _isNoise;};
  void setIsNoise(bool i){_isNoise=i;}
private:
  char* _ptr;
  TdcChannel* _Tdcptr;
  TdcChannel* _end;
  bool _isNoise{false};
};
#endif
