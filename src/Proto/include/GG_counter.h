#ifndef GG_counter_HH
#define GG_counter_HH

#include <map>
#include <iostream>
#include <string>
#include "ASCIIpersistance.h"

class SingleCounter : public ASCIIpersistance
{
 public:
 SingleCounter() : m_count(0), m_flagcount(0), m_flag(false) {;}

  void add(unsigned int val=1, unsigned int *unused=NULL) {m_count+=val;  if (!m_flag) {++m_flagcount;m_flag=true;}}
  void newSet() {m_flag=false;}
  
  unsigned int sumcount() const {return m_count;}
  unsigned int flagcount() const {return m_flagcount;}

  void write(std::string* labels=NULL,std::ostream& oflux=std::cout) const { oflux <<  (labels==NULL ? std::string("") : (*labels))  << " : " <<  m_count << " for " << m_flagcount << std::endl;}

  bool ASCIIwrite(std::ostream& oflux=std::cout) const { oflux << m_count << " " << m_flagcount<< " " ; return oflux.good();}
  bool ASCIIread(std::istream& iflux=std::cin) {iflux >> m_count >> m_flagcount; return iflux.good();}

  bool operator==(const SingleCounter& other) const {return sumcount()==other.sumcount() && flagcount()==other.flagcount();}
  bool operator!=(const SingleCounter& other) const {return ! ((*this)==other);}

  static const unsigned int printIndentLevel=0;

  SingleCounter& counterAtLevel(unsigned int level, unsigned int *) {return *this;} 

 private:
  unsigned int m_count;
  unsigned int m_flagcount;
  bool m_flag;
};


class SingleMapCounter : public ASCIIpersistance
{
 public:
 SingleMapCounter() : m_valdistribution() {}

  void add(unsigned int val=1, unsigned int *unused=NULL) {++m_valdistribution[val];}
  void newSet() {;}

  const std::map<unsigned int,unsigned int>& distribution() const {return m_valdistribution;}

  void write(std::string* labels=NULL,std::ostream& oflux=std::cout) const
  {
    oflux <<  (labels==NULL ? std::string("") : (*labels))  << " : ";
    for (std::map<unsigned int,unsigned int>::const_iterator it=m_valdistribution.begin(); it != m_valdistribution.end(); ++it) oflux << " " << it->first << "::" << it->second;
    oflux << std::endl;
  }

  bool ASCIIwrite(std::ostream& oflux=std::cout) const
  {
    oflux <<  m_valdistribution.size() << " ";
    for (std::map<unsigned int,unsigned int>::const_iterator it=m_valdistribution.begin(); it != m_valdistribution.end(); ++it)
      oflux << it->first << " " << it->second << " ";
    return oflux.good();
  }
  bool ASCIIread(std::istream& iflux=std::cin)
  {
    m_valdistribution.clear();
    unsigned int mapsize,key,val;
    iflux >> mapsize;
    for (unsigned int i=0; i<mapsize; ++i) {iflux >> key >> val; m_valdistribution[key]=val;} 
    return iflux.good();
  }
  
  bool operator==(const SingleMapCounter& other) const {return m_valdistribution==other.distribution();}
  bool operator!=(const SingleMapCounter& other) const {return ! ((*this)==other);}

  static const unsigned int printIndentLevel=0;

  SingleMapCounter& counterAtLevel(unsigned int level, unsigned int *) {return *this;} 

 private:
  std::map<unsigned int,unsigned int> m_valdistribution;
};


class TDC_EffCounter : public ASCIIpersistance
{
 public:
 TDC_EffCounter() : m_positive(0), m_event(0), m_ntriggerSeenInThisEvent(0), m_maxtriggerToSeeInThisEvent(0) {;}

  void add(unsigned int val=1, unsigned int *unused=NULL) {m_ntriggerSeenInThisEvent+=val; ++m_maxtriggerToSeeInThisEvent; }
  void newSet() {
    if (m_maxtriggerToSeeInThisEvent>0) ++m_event; m_maxtriggerToSeeInThisEvent=0;
    if (m_ntriggerSeenInThisEvent>0) ++m_positive; m_ntriggerSeenInThisEvent=0;
  }
  
  unsigned int sumcount() const {return m_positive;}
  unsigned int flagcount() const {return m_event;}

  void write(std::string* labels=NULL,std::ostream& oflux=std::cout) const { oflux <<  (labels==NULL ? std::string("") : (*labels))  << " : " <<  m_positive << " for " << m_event << ". Eff= " << (100.0*m_positive)/m_event << " %" << std::endl;}

  bool ASCIIwrite(std::ostream& oflux=std::cout) const { oflux << m_positive << " " << m_event<< " " ; return oflux.good();}
  bool ASCIIread(std::istream& iflux=std::cin) {iflux >> m_positive >> m_event; return iflux.good();}

  bool operator==(const SingleCounter& other) const {return sumcount()==other.sumcount() && flagcount()==other.flagcount();}
  bool operator!=(const SingleCounter& other) const {return ! ((*this)==other);}

  static const unsigned int printIndentLevel=0;

  TDC_EffCounter& counterAtLevel(unsigned int level, unsigned int *) {return *this;} 

 private:
  unsigned int m_positive;
  unsigned int m_event;
  unsigned int m_ntriggerSeenInThisEvent;
  unsigned int m_maxtriggerToSeeInThisEvent;
};



//COUNTER should derive form COUNTERBASE
template <class COUNTER, class COUNTERBASE=SingleCounter>
class MappedCounters : public std::map<unsigned int,COUNTER>, public COUNTERBASE
{
 public:
  void add(unsigned int val, unsigned int *keys) {COUNTERBASE::add(val); (*this)[keys[0]].add(val,keys+1); }
  void newSet() {COUNTERBASE::newSet(); for (typename std::map<unsigned int,COUNTER>::iterator it=this->begin(); it!= this->end(); ++it) it->second.newSet();}
 
  COUNTERBASE& counterAtLevel(unsigned int level, unsigned int *keys) 
    {
      if (level>=printIndentLevel) return COUNTERBASE::counterAtLevel(level,keys);
      return (*this)[keys[0]].counterAtLevel(level,keys+1);
    }

  void write(std::string* labels,std::ostream& oflux=std::cout) 
  {
    COUNTERBASE::write(labels,oflux); 
    for (typename std::map<unsigned int,COUNTER>::iterator it=this->begin(); it!= this->end(); ++it) {printIndent(oflux); oflux << it->first << " "; it->second.write(labels+1,oflux);}
  }

  bool ASCIIwrite(std::ostream& oflux=std::cout) const
  { 
    COUNTERBASE::ASCIIwrite(oflux);  
    oflux << this->size() << " ";
    for (typename std::map<unsigned int,COUNTER>::const_iterator it=this->begin(); it!= this->end(); ++it) 
      { oflux << it->first << " "; it->second.ASCIIwrite(oflux); }
    return oflux.good();
  }
  bool ASCIIread(std::istream& iflux=std::cin)
  {
    this->clear();
    COUNTERBASE::ASCIIread(iflux);
    unsigned int mapsize;
    iflux >> mapsize;
    for (unsigned int i=0; i<mapsize; ++i)
      { unsigned int aKey; iflux >> aKey; (*this)[aKey].ASCIIread(iflux); }
    return iflux.good();
  }

  const std::map<unsigned int,COUNTER>& the_map() const {return *this;}
  bool operator==(const MappedCounters<COUNTER>& other) const {return  COUNTERBASE::operator==(other) && the_map()==other.the_map();}
  bool operator!=(const MappedCounters<COUNTER>& other) const {return ! ((*this)==other);}

  static const unsigned int printIndentLevel=COUNTER::printIndentLevel+1;
 private:
  static void printIndent(std::ostream& oflux) { for (unsigned int i=printIndentLevel; i<5;  ++i) oflux << "  ";}
  
}; 

typedef MappedCounters<TDC_EffCounter,TDC_EffCounter> MezzanineCounters;
typedef MappedCounters<MezzanineCounters,TDC_EffCounter> ChamberCounters;


#endif
