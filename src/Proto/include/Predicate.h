#ifndef Predicate_h
#define Predicate_h 
class TdcChannelBcidpredicate
{
public:
  TdcChannelBcidpredicate(int bcid, int lowShift, int highShift) : _trigBCID(bcid), _lowbcid(bcid+lowShift),   _highbcid(bcid+highShift ){}
  bool operator()(TdcChannel& c) {uint16_t bcid=c.bcid(); return ((bcid>=_lowbcid && bcid <=_highbcid)||(bcid==_trigBCID && c.channel()==triggerChannel));} 
private:
  int _trigBCID;
  int _lowbcid;
  int _highbcid;
};

class TdcChannelMezzaninePredicate
{
public:
  TdcChannelMezzaninePredicate(int chamber, int mezzanine) : _chamber(chamber), _mezzanine(mezzanine) {}
  bool operator()(TdcChannel& c) {return c.mezzanine()==_mezzanine && c.chamber()==_chamber;}
private:
  int _chamber;
  int _mezzanine;
};
#endif
