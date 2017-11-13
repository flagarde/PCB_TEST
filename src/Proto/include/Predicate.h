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
    //bool tdcTimeConditions=(_trigtdcTime-c.tdcTime()>=-50000&&_trigtdcTime-c.tdcTime()<=50000);
    //return bcidConditions&&tdcTimeConditions;
    return bcidConditions;
  } 
private:
  int _trigBCID;
  double _trigtdcTime;
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

class TdcOutofTriggerTimePredicate
{
 public:
   TdcOutofTriggerTimePredicate(double trigTime,double shiftLowSide, double shiftUpSide) : _lowSide(trigTime+shiftLowSide), _highSide(trigTime+shiftUpSide) {}
  bool operator()(TdcChannel& c) {double t=c.tdcTime(); return t<_lowSide || t>_highSide;}
 private:
  double _lowSide;
  double _highSide;
};

#endif
