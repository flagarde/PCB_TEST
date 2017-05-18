#ifndef Predicate_h
#define Predicate_h 
class TdcChannelBcidpredicate
{
public:
  TdcChannelBcidpredicate(int bcid,double tdcTime, int lowShift, int highShift) : _trigBCID(bcid),_trigtdcTime(tdcTime), _lowbcid(bcid+lowShift),   _highbcid(bcid+highShift ){}
  bool operator()(TdcChannel& c) 
  {
    uint16_t bcid=c.bcid(); 
    bool bcidConditions=(bcid>=_lowbcid && bcid <=_highbcid)||(c.channel()==triggerChannel && (bcid==_trigBCID || bcid==_trigBCID+1 || bcid+1==_trigBCID));
    bool tdcTimeConditions=(_trigtdcTime-c.tdcTime()>=-50000&&_trigtdcTime-c.tdcTime()<=50000);
    return bcidConditions&&tdcTimeConditions;
  } 
private:
  int _trigBCID;
  double _trigtdcTime;
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
