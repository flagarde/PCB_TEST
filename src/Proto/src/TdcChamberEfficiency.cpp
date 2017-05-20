#include "TdcChamberEfficiency.hh"

#include <iostream>

void TdcChamberEfficiency::accumulate(std::map<unsigned int, channelSeen>& m)
{
  channelSeen &A=m[m_mezzanineA];
  channelSeen &B=m[m_mezzanineB];
  if (A.m_triggerSeen && B.m_triggerSeen)       {++m_ntriggerAandB; m_hittriggerAandB.accumulate(A,B);}
  if (A.m_triggerSeen && (not B.m_triggerSeen)) {++m_ntriggerAonly; m_hittriggerAonly.accumulate(A,B);}
  if ((not A.m_triggerSeen) && B.m_triggerSeen) {++m_ntriggerBonly; m_hittriggerBonly.accumulate(A,B);}
  if ((not A.m_triggerSeen) && (not B.m_triggerSeen)) {++m_ntriggerNobody; m_hittriggerNobody.accumulate(A,B);}
}

void TdcChamberEfficiency::hitCounter::accumulate(channelSeen &A, channelSeen &B)
{
  if (A.m_hitSeen && B.m_hitSeen) ++n_HitinAandB;
  if (A.m_hitSeen && (not B.m_hitSeen)) ++n_HitinAonly;
  if ((not A.m_hitSeen) && B.m_hitSeen) ++n_HitinBonly;
  if ((not A.m_hitSeen) && (not B.m_hitSeen)) ++n_HitNobody;
}

void TdcChamberEfficiency::printeff(unsigned int numerator, unsigned int denominator)
{
  std::cout << numerator << " / " << denominator << " = " << (100.0*numerator)/denominator << std::endl;
}
  
void TdcChamberEfficiency::print()
{
  std::cout << " Efficiency stat for chamber made of mezzanines " <<  m_mezzanineA << " and " << m_mezzanineB << "." << std::endl;
  std::cout << " sample A and B triggered, size= " <<  m_ntriggerAandB;  m_hittriggerAandB.print();
  std::cout << " sample A only triggered, size= " <<  m_ntriggerAonly;  m_hittriggerAonly.print();
  std::cout << " sample B only triggered, size= " <<  m_ntriggerBonly;  m_hittriggerBonly.print();
  std::cout << " sample nobody triggered, size= " <<  m_ntriggerNobody;  m_hittriggerNobody.print();

  std::cout << " Efficiency mezzanine " << m_mezzanineA << " :  "; printeff(n_hitInA_ifAtriggered(),n_triggerinA());
  std::cout << " Efficiency mezzanine " << m_mezzanineB << " :  "; printeff(n_hitInB_ifBtriggered(),n_triggerinB());
  std::cout << " Efficiency chamber  :  "; printeff(n_hitSomewhere_ifAnytriggered(),n_triggerSomewhere());
  
  std::cout << " Efficiency mezzanine " << m_mezzanineA << " trigger in both :  "; printeff(m_hittriggerAandB.n_hitinA(),m_ntriggerAandB);
  std::cout << " Efficiency mezzanine " << m_mezzanineB << " trigger in both :  "; printeff(m_hittriggerAandB.n_hitinB(),m_ntriggerAandB);
  std::cout << " Efficiency chamber trigger in both :  "; printeff(m_hittriggerAandB.n_hitSomewhere(),m_ntriggerAandB);
}

void TdcChamberEfficiency::hitCounter::print()
{
  std::cout << " dispatched in "
	    <<  n_HitinAandB << " with hit in A and B, "
	    <<  n_HitinAonly << " with hit only in A " 
	    <<  n_HitinBonly << " with hit only in B " 
	    <<  n_HitNobody << " without hit " << std::endl;
}

void TdcMultiChamberEfficiency::print()
{
  for (unsigned int i=0; i<m_efficiencyVector.size(); ++i)
    {
      std::cout << "Report efficiency for chamber " << i+1 << std::endl;
      m_efficiencyVector[i].print();
    }
}
