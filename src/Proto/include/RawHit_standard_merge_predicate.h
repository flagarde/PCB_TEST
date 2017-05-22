#ifndef RawHit_standard_merge_predicate_HH
#define RawHit_standard_merge_predicate_HH
#include "TdcChannels.hh"

class RawHit_standard_merge_predicate
{
 public:
  RawHit_standard_merge_predicate(int trigger):m_neighbourTimeDistance(1), m_neighbourStripDistance(2),triggerChannel(trigger){};
  unsigned int getNeighbourTimeDistance() const {return m_neighbourTimeDistance;}
  unsigned int getNeighbourStripDistance() const {return m_neighbourStripDistance;}
  void setNeighbourTimeDistance(unsigned int val) {m_neighbourTimeDistance=val;}
  void setNeighbourStripDistance(unsigned int val) {m_neighbourStripDistance=val;}
  void setSide(unsigned int val)
  {
	  if(val>1) m_side=2;
	  else if(val==1) m_side=1;
	  else if(val==0) m_side=0;
  }
  bool operator()(TdcChannel* A,TdcChannel* B);
 private:
  unsigned int m_neighbourTimeDistance;
  unsigned int m_neighbourStripDistance;
  unsigned int m_side;
  bool IJ_connect(int I1, int I2);
  bool SideConnect(int side1,int side2);
  int triggerChannel;
};
#endif
