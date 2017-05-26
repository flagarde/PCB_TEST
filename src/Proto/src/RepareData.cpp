#include <cstdint>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "buffer.h"
#include "TdcChannels.hh"

int main(int argc, char *argv[])
{
  std::string filename=argv[1];
  int32_t fdIn= ::open(filename.c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
  
  if (fdIn<0)
    {
      perror("Ici No way to read file :");
      return 1;
    }
  filename.insert(filename.rfind("."),"_streamed");
  std::cout << filename << std::endl;
  int32_t fdOut= ::open(filename.c_str(), O_WRONLY | O_CREAT ,S_IRWXU);
  if (fdOut<0)
    {
      perror("Ici No way to store to file :");
      return 1;
    }  
  uint32_t event,run,totalSize;
  uint64_t bxId;
  uint32_t gtc,runType,dacSet,vthSet/*,mezzanine*/,difId;
  std::map<int32_t,std::vector<levbdim::buffer*> > bmap;
  
  while (true)
    {
      uint32_t theNumberOfDIF=0;
      int ier=::read(fdIn,&event,sizeof(uint32_t));
      if (ier<=0) { printf("Cannot read anymore %d \n ",ier);return 0;}
      ier=::read(fdIn,&theNumberOfDIF,sizeof(uint32_t));
      if (ier<=0) { printf("Cannot read anymore number of DIF %d \n ",ier);return 0;}
      for (uint32_t idif=0;idif<theNumberOfDIF;idif++)
	{
	  uint32_t bsize=0;
	  ier=::read(fdIn,&bsize,sizeof(uint32_t));
	  if (ier<=0){ printf("Cannot read anymore  DIF Size %d \n ",ier);return 0;}
	  levbdim::buffer* b= new levbdim::buffer(0x100000);
	  ier=::read(fdIn,b->ptr(),bsize);
	  if (ier<=0){printf("Cannot read anymore Read data %d \n ",ier);return 0;}
	  b->setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
	  b->uncompress();
	  bxId=b->bxId();
	  if (b->detectorId()==255)
	    {
	      uint32_t* buf=(uint32_t*) b->payload();
	      printf("NEW RUN %d \n",event);
	      run=event;
	      difId=b->dataSourceId();
	      runType=buf[0];
	      if (runType==1)
		dacSet=buf[1];
	      if (runType==2)
		vthSet=buf[1];
	      printf("\n Run type %d DAC set %d VTH set %d \n",runType,dacSet,vthSet); 
	    } //end detId==255
	  if (b->detectorId()==110)
	    {
	      uint32_t* ibuf=(uint32_t*) b->payload();
	      uint32_t nch=ibuf[6],chtrg=16;
	      uint16_t curbcid=0,curgtc=0;
	      gtc=ibuf[1];
	      if (nch>0)
		{
		  uint8_t* cbuf=( uint8_t*)&ibuf[7];
		  for (int i=0;i<nch;i++)
		    {
		      TdcChannel c(&cbuf[8*i]);
		      if (c.channel()==chtrg)
			{
			  curbcid=c.bcid();
			  curgtc=gtc;
			  break;
			}
		    }// loop on channel
		} // if nch>0
	      if (curbcid==0 || curgtc==0)
		{
		  //suppress if no trigger found
		  delete b;
		  continue;
		}
	      // All possible id
	      std::vector<uint32_t> vids;
	      for (int i=-1;i<=1;i++)
		for (int j=-1;j<=1;j++)
		  {
		    int32_t ig=curgtc+i;
		    int32_t jb=curbcid+j;
		    vids.push_back( (ig<<16)|jb);
		  }
	      // Look for one id in bmap
	      bool append=false;
	      for (auto id:vids)
		{
		  if (bmap.find(id)!=bmap.end())
		    {
		      append=true;
		      bmap.find(id)->second.push_back(b);
		      break;
		    }
		}
	      if (!append)
		{
		  std::vector<levbdim::buffer*> v;
		  v.push_back(b);
		  bmap.insert(std::pair<int32_t,std::vector<levbdim::buffer*> >( ((curgtc<<16)|curbcid),v));
		}
	    }//end detId==110
	} //loop on idif
      // Now loop on buffer MAP and check full event
      uint32_t numberOfDIFforFullEvent=4;
      for ( std::map<int32_t,std::vector<levbdim::buffer*> >::iterator it=bmap.begin();it!=bmap.end();it++)
	{
	  if (it->second.size()!=numberOfDIFforFullEvent) continue;
	  if (it->first==0) continue; // do not process event 0
	  event=((it->first)>>16)& 0xFFFF;
	  std::cout<<"full  event find  Trigger " <<((it->first)& 0xFFFF)<<" GTC  "<< (((it->first)>>16)& 0xFFFF)<<std::endl;
	  uint32_t theNumberOfDIF=it->second.size();
	  if (fdOut>0)
	    {
	      int ier=write(fdOut,&event,sizeof(uint32_t));
	      ier=write(fdOut,&theNumberOfDIF,sizeof(uint32_t));
	      for (std::vector<levbdim::buffer*>::iterator iv=it->second.begin();iv!=it->second.end();iv++)
		{
		  (*iv)->compress();
		  uint32_t bsize=(*iv)->size();
		  totalSize+=bsize;
		  ier=write(fdOut,&bsize,sizeof(uint32_t));
		  ier=write(fdOut,(*iv)->ptr(),bsize);
		}
	    } // if fdout>0
	}// end bmap loop
      // remove completed events
      for (std::map<int32_t,std::vector<levbdim::buffer*> >::iterator it=bmap.begin();it!=bmap.end();)
	{
	  if (it->second.size()==numberOfDIFforFullEvent)
	    {
	      for (std::vector<levbdim::buffer*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) delete (*iv);
	      it->second.clear();
	      bmap.erase(it++);
	    }
	  else
	    it++;
	}//second bmap loop
    } // while loop
  return 0;
}
