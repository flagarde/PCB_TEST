#ifndef TdcChamberEfficiency_HH
#define TdcChamberEfficiency_HH

#include <map>
#include <vector>


class channelSeen
{
public:
  channelSeen() {reset();}
  void reset() {m_triggerSeen=m_hitSeen=false;}
  bool m_triggerSeen;
  bool m_hitSeen;
};

inline void reset(std::map<unsigned int, channelSeen> &m) { for (auto it=m.begin(); it!=m.end(); ++it) it->second.reset();}


class TdcChamberEfficiency
{
public:
  TdcChamberEfficiency( unsigned int mezzanineA, unsigned int mezzanineB) : m_mezzanineA(mezzanineA), m_mezzanineB(mezzanineB) {}
  void accumulate(std::map<unsigned int, channelSeen>&);
  void print();
private:
  class hitCounter
  {
  public:
    void accumulate(channelSeen &A, channelSeen &B);
    unsigned int n_HitinAandB=0;
    unsigned int n_HitinAonly=0;
    unsigned int n_HitinBonly=0;
    unsigned int n_HitNobody=0;
    void print();
    unsigned int n_hitinA() {return n_HitinAandB+n_HitinAonly;}
    unsigned int n_hitinB() {return n_HitinAandB+n_HitinBonly;}
    unsigned int n_hitSomewhere() {return n_HitinAandB+n_HitinAonly+n_HitinBonly;}

  };

  unsigned int m_mezzanineA;
  unsigned int m_mezzanineB;

  unsigned int m_ntriggerAandB=0;
  unsigned int m_ntriggerAonly=0;
  unsigned int m_ntriggerBonly=0;
  unsigned int m_ntriggerNobody=0;

  hitCounter m_hittriggerAandB;
  hitCounter m_hittriggerAonly;
  hitCounter m_hittriggerBonly;
  hitCounter m_hittriggerNobody;

  unsigned int n_hitInA_ifAtriggered() {return m_hittriggerAandB.n_hitinA() + m_hittriggerAonly.n_hitinA(); }
  unsigned int n_triggerinA() { return m_ntriggerAandB+m_ntriggerAonly;}
  unsigned int n_hitInB_ifBtriggered() {return m_hittriggerAandB.n_hitinB() + m_hittriggerBonly.n_hitinB(); }
  unsigned int n_triggerinB() { return m_ntriggerAandB+m_ntriggerBonly;}

  unsigned int n_hitSomewhere_ifBothtriggered() {return m_hittriggerAandB.n_hitSomewhere();}
  unsigned int n_hitSomewhere_ifAnytriggered() {return m_hittriggerAandB.n_hitSomewhere()+m_hittriggerAonly.n_hitSomewhere()+m_hittriggerBonly.n_hitSomewhere();}
  unsigned int n_triggerSomewhere() {return m_ntriggerAandB+m_ntriggerAonly+m_ntriggerBonly;}

  void printeff(unsigned int numerator, unsigned int denominator);
};


class TdcMultiChamberEfficiency
{
public:
  void addChamber(unsigned int mezzA,unsigned int mezzB) {m_efficiencyVector.push_back(TdcChamberEfficiency(mezzA,mezzB));}
  void startEvent() { reset(m_seen); }
  void setTriggerSeen(unsigned int mezzanine) { m_seen[mezzanine].m_triggerSeen=true;}
  void setHitSeen(unsigned int mezzanine) { m_seen[mezzanine].m_hitSeen=true;}
  void endEvent() {    for (auto it=m_efficiencyVector.begin(); it !=m_efficiencyVector.end(); ++it) it->accumulate(m_seen);}
  void print();
private:
  std::map<unsigned int, channelSeen> m_seen;
  std::vector<TdcChamberEfficiency> m_efficiencyVector;
};
#endif
