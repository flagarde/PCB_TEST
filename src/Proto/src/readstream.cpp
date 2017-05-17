#include "readstream.h"
#include "TdcChannels.hh"
int readstream(int32_t _fdIn,ReadoutProcessor &ro_processor)
{
  if (_fdIn<=0) return 0;
  uint32_t _event=0;
  uint32_t maxsize=0x100000;
  levbdim::buffer b(maxsize);
  TdcChannelBuffer tdcBuf(maxsize);
  while (true)
  {
    tdcBuf.clear();
    uint32_t theNumberOfDIF=0;
    int ier=::read(_fdIn,&_event,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore %d \n ",ier); return 0;
	  }
    else printf("Event read %d \n",_event);
    if(numbereventtoprocess==_event)return 0;
    ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
    if (ier<=0)
	  {
	    printf("Cannot read anymore number of DIF %d \n ",ier); return 0;
	  }
    //else
    //printf("Number of DIF found %d \n",theNumberOfDIF);
    for (uint32_t idif=0;idif<theNumberOfDIF;idif++) 
	  {
	    uint32_t bsize=0;
	    ier=::read(_fdIn,&bsize,sizeof(uint32_t));
	    if (ier<=0)
	    {
	      printf("Cannot read anymore  DIF Size %d \n ",ier);return 0;
	    }
	    ier=::read(_fdIn,b.ptr(),bsize);
	    if (ier<=0)
	    {
	      printf("Cannot read anymore Read data %d \n ",ier);return 0;
	    }
	    b.setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
	    b.uncompress();
	    if (b.detectorId() != 110) continue;
	    uint32_t* ibuf=(uint32_t*) b.payload();
	    //for (int i=0;i<7;i++)  printf("%d ",ibuf[i]);
	    //uint64_t absbcid=ibuf[3]; absbcid=(absbcid<<32)|ibuf[2];
	    //printf("\n event number %d, GTC %d, ABSBCID %lu, mezzanine number %u, ",ibuf[0],ibuf[1]&0xFFFF,absbcid,ibuf[4]);
	    //printf("IP address %u.%u.%u.%u,",ibuf[5]&0xFF,(ibuf[5]>>8)&0xFF,(ibuf[5]>>16)&0xFF,(ibuf[5]>>24)&0xFF);
	    //uint32_t nch=ibuf[6];
	    //printf("\n channels -> %d \n",nch);
	    if (ibuf[6]>0)
	    {
	      uint8_t* cbuf=( uint8_t*)&ibuf[7];
	      for (int i=0;i<ibuf[6];i++)
		    {
		      tdcBuf.addChannel(&cbuf[8*i]);
		      TdcChannel &c=tdcBuf.last();
		      c.setstrip(ibuf[4],(ibuf[5]>>24)&0xFF);
		    }
	    }
	  } // end loop on DIF
    ro_processor.processReadout(tdcBuf);
  }
} 
