#include "RawHit_standard_merge_predicate.h"
#include <iostream>
#include "TdcChannels.hh"
bool RawHit_standard_merge_predicate::operator()(TdcChannel *A,TdcChannel *B)
{
  if (abs(A->getTimeFromTrigger()-B->getTimeFromTrigger())>m_neighbourTimeDistance) return false;
  if (triggerChannel==A->strip()||triggerChannel==B->strip())return false;
  if ((A->chamber())!=(B->chamber()))return false;
  return IJ_connect(A->strip()%100,B->strip()%100)&&SideConnect(A->side(),B->side());
}

bool RawHit_standard_merge_predicate::IJ_connect(int I1, int I2)
{
  unsigned int distI=abs(I1-I2);
  return (distI <= m_neighbourStripDistance);
}

bool RawHit_standard_merge_predicate::SideConnect(int side1,int side2)
{
  if(m_side<=1) return side1==m_side&&side2==m_side;
  else return true;
}

