#ifndef SDHCAL_Clustering_HH
#define SDHCAL_Clustering_HH
#include <algorithm>
#include <ostream>
#include <set>
#include <vector>

 template <class T> 
  class Cluster : public std::set<const T*>
  {
    public:
    Cluster() : std::set<const T*>() {}
    Cluster(const T& obj) : std::set<const T*>() {add(obj);}
    void add(const T& obj) {this->insert(&obj);}
    void merge(const Cluster<T>& cl) {this->insert(cl.begin(),cl.end());}
  };

  template <class iterT,class mergePred>
  iterT buildOneCluster(iterT first, iterT last,  mergePred pred)
  {
    iterT itCurrentClusterEnd=first;
    ++itCurrentClusterEnd;
    iterT itOtherElement=itCurrentClusterEnd;
    while (itOtherElement != last)
    {
	    for (iterT itCheck=first; itCheck != itCurrentClusterEnd; ++itCheck)
	    {
	      if (pred(*itOtherElement,*itCheck)) //itOtherElement should be added to current cluster
	      {
		      if (itOtherElement==itCurrentClusterEnd) ++itCurrentClusterEnd;
		      else
		      {
		        std::iter_swap(itOtherElement,itCurrentClusterEnd);
		        itCurrentClusterEnd=buildOneCluster(itCurrentClusterEnd,last,pred);
		        itOtherElement=itCurrentClusterEnd;
		        --itOtherElement;
		      }
		      break;
	      }
	    }
	    ++itOtherElement;	  
    }
    return itCurrentClusterEnd;
  }

  template <class iterT,class mergePred>
  void clusterize(iterT first, iterT last, std::vector<iterT> &clusterBounds, mergePred pred)
  {
    clusterBounds.push_back(first);
    if (first != last) clusterize(buildOneCluster(first,last,pred),last,clusterBounds,pred);
  }
  
  template <class T, class iterT>
  void Convert(const std::vector<iterT> &clusterBounds,std::vector<Cluster<T> >& cl)
  {
    for (int i=0; i<clusterBounds.size()-1; ++i)
    {
	    Cluster<T> C;
	    for (iterT itHits=clusterBounds[i]; itHits!=clusterBounds[i+1]; ++itHits) C.add(*itHits);
	    cl.push_back(C);
    }    
  }
    
  template <class T, class iterT, class mergePred>
  void clusterize(iterT first, iterT last,std::vector<Cluster<T> >& cl, mergePred pred)
  {
    std::vector<iterT> clusterBounds;
    clusterize(first,last,clusterBounds,pred);
    Convert(clusterBounds,cl);
  }

//
//  Example of usage :
//  assuming f is a function bool f(const int &,const int &);
//  int tab[15]; 
//  //fill tab
//  std::vector<int*> clusters;
//  SDHCAL::clusterize((int*) tab, tab+15,clusters,f);
//  
//  clusters contains the cluster bounds
#endif
