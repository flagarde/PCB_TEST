#include "RawHit_standard_merge_predicate.h"
#include <iostream>
#include "TdcChannels.hh"
bool RawHit_standard_merge_predicate::operator()(TdcChannel *A,TdcChannel *B)
{
  if (abs(A->getTimeFromTrigger()-B->getTimeFromTrigger())>m_neighbourTimeDistance) return false;
  if (triggerChannel==A->strip()||triggerChannel==B->strip())return false;
  if ((A->chamber())!=(B->chamber()))return false;
  if(m_side>1&&((A->chamber())!=m_side&&(B->chamber())!=m_side)) return false;
  return IJ_connect(A->strip()%100,B->strip()%100);
}

bool RawHit_standard_merge_predicate::IJ_connect(int I1, int I2)
{
  unsigned int distI=abs(I1-I2);
  return distI <= m_neighbourStripDistance;
}
