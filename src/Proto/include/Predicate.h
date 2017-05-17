#ifndef Predicate_h
#define Predicate_h 
class TdcChannelBcidpredicate
{
public:
  TdcChannelBcidpredicate(int bcid, int lowShift, int highShift) : _trigBCID(bcid), _lowbcid(bcid+lowShift),   _highbcid(bcid+highShift ){}
  bool operator()(TdcChannel& c) {uint16_t bcid=c.bcid(); return ((bcid>=_lowbcid && bcid <=_highbcid)
								  ||(c.channel()==triggerChannel && (bcid==_trigBCID || bcid==_trigBCID+1 || bcid+1==_trigBCID) ));} 
private:
  int _trigBCID;
  int _lowbcid;
  int _highbcid;
};

class TdcMezzaninePredicate
{
public:
  TdcMezzaninePredicate(int mezzanine) : _mezzanine(mezzanine) {}
  bool operator()(TdcChannel& c) {return c.mezzanine()==_mezzanine;}
private:
  int _mezzanine;
};
#endif
