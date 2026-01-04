#ifndef Stage1Passthrough_h
#define Stage1Passthrough_h

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <random>

#include "Stage1IO.hh"
#include "Stage1PassthroughFwCfg.hh"

namespace TPGStage1Emulation {

  class Stage1Passthrough : public Stage1IO {
public:
  Stage1Passthrough() : _runCall(0) {
  }

  // Input D = unpacker output
  virtual void runAlgorithm() {

    bool doPrint(false);
    if(doPrint) std::cout << "Entering run: chain Z, BX = "
			  << unsigned(_vUos[0][0].bx(0)) << std::endl;
    
    uint64_t tcPattern[2]={0xabcddeed,0xbeeffeed};
    uint64_t pttPattern(0xabcddeedfeedbeef);

    //uint64_t tcData[4][108]; // Only use 42 bits per word
    //uint64_t pttData[36]; // Only use 48 bits per word

    //uint64_t crc(0);

    //bool leadingBit((_runCall/18)%2==1);
    
    
    for(unsigned link(0);link<4;link++) {
      for(unsigned frame(0);frame<108;frame++) {
	_packerInputTc[link][frame][0]=0;
	_packerInputTc[link][frame][1]=0;
	_packerInputTc[link][frame][2]=0;
      }
      unsigned nTc(0);

      for(unsigned lp(0);lp<Stage1IOFwCfg::MaximumLpgbtPairs && nTc<300;lp++) {
	if(_cfgPtPtr->outputLink(lp)==link) {

	  // TCs
      
	  for(unsigned i(0);i<2;i++) {
	    for(unsigned up(0);up<Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair && nTc<300;up++) {
	      const uint16_t *pData(_vUos[lp][up].getData(i));

	      for(unsigned n(0);n<_cfgPtPtr->outputNumberOfTCs(lp,up) && n<3 && nTc<300;n++) {
		/*
		uint16_t ea(0x8000);
		if(n==0) ea=_vUos[lp][up][i].getMsData();
		else     ea=_vUos[lp][up][i].getTcData(n-1);
		*/
		uint16_t ea(*(pData+n));
		
		//ea=0xffff;
		
		//uint16_t ea(vUos[nUos].channelEnergy(nUosCh));
		//uint16_t ea(vUos[nUos][nUosCh/3].getTcData(nUosCh));
		//if(vUos[nUos][nUosCh/3].numberOfValidChannels()==0) ea=0;
		//uint16_t ea(nUosCh%3);
		//uint16_t ea(0x8000);
		
		//for(unsigned bit(0);bit<15;bit++) {
		//unsigned nBit(15*val+bit);

		/*
		for(unsigned bit(0);bit<15;bit++) {
		  unsigned nBit(15*nTc+bit);
		  if((ea&(1<<bit))!=0) _packerInputTc[link][nBit/42]|=uint64_t(1)<<(nBit%42);
		  if(doPrint) 
		    std::cout << "Dumping nTC = " << std::dec << std::setw(3) << nTc << " " << nBit
			      << " 0x" << std::hex << std::setw(16) << _packerInputTc[link][nBit/42]
			      << std::endl;
		}
		*/
		_packerInputTc[link][nTc/3][nTc%3]=ea;
		nTc++;
	      }
	    }
	  }
	}
      }

      _packerInputTc[link][107][0]=(tcPattern[link%2]<<13)&0x7fff;
      _packerInputTc[link][107][1]=(tcPattern[link%2]>> 2)&0x7fff;
      _packerInputTc[link][107][2]=(tcPattern[link%2]>>17)&0x7fff;
    }
      
    // pTTs
    for(unsigned link(0);link<6;link++) {
      _packerInputPtt[link][0][0]=(pttPattern    )&0xff;
      _packerInputPtt[link][0][1]=(pttPattern>> 8)&0xff;
      _packerInputPtt[link][1][0]=(pttPattern>>16)&0xff;
      _packerInputPtt[link][1][1]=(pttPattern>>24)&0xff;
      _packerInputPtt[link][2][0]=(pttPattern>>32)&0xff;
      _packerInputPtt[link][2][1]=(pttPattern>>40)&0xff;
      _packerInputPtt[link][3][0]=(pttPattern>>48)&0xff;
      _packerInputPtt[link][3][1]=(pttPattern>>56)&0xff;

      for(unsigned frame(4);frame<108;frame++) {
	_packerInputPtt[link][frame][0]=0;
	_packerInputPtt[link][frame][1]=0;
      }
    }
    /*    
    // Copy bits in right order

    for(unsigned link(0);link<6;link++) {
      std::array<uint64_t,108> &d(vS12[link]->accessData());
      //std::array<uint64_t,108> d;

      for(unsigned frame(0);frame<108;frame++) {
	if(leadingBit) d[frame]=(uint64_t(0x2)<<62)|crc;
	else d[frame]=crc;
      }

      unsigned tcLink(link);
      if(link==2) tcLink=1;
      if(link==3) tcLink=2;
      if(link==4) tcLink=3;
      if(link==5) tcLink=3;
      
      bool distort(false);
      for(unsigned group(0);group<18;group++) {
	for(unsigned sub(0);sub<6;sub++) {
	
	  // TCs
	  if(distort) {
	    for(unsigned bit(0);bit<42;bit++) {
	      unsigned nBit(42*sub+bit);
	      if((tcData[tcLink][6*group+sub]&(uint64_t(1)<<bit))!=0) d[102-6*group+((nBit+7)%6)]|=uint64_t(1)<<((nBit/6)+20);
	    }
	  } else {
	    d[102-6*group+sub]|=tcData[tcLink][6*group+sub]<<20;
	  }
	}	  
      }

      // pTTs
      for(unsigned group(0);group<6;group++) {
	for(unsigned sub(0);sub<6;sub++) {
	  if(distort) {
	    for(unsigned bit(0);bit<48;bit++) {
	      unsigned nBit(48*sub+bit);
	      if((pttData[6*group+sub]&(1<<bit))!=0) d[102-18*group+((nBit+7)%6)-6*(bit/16)]|=uint64_t(1)<<(((nBit/6)%16)+4);
	    }
	  } else {
	    d[102-6*group+sub]|=uint64_t(pttData[6*group+sub])<<4;
	  }
	}
      }
      
      // TC patterns
      //vS12[link]->accessData()=d;
    }

    _runCall++;
    */
    
    if(doPrint) std::cout << "Exiting  run: chain Z" << std::endl;  
  }

  void setHardwareConfiguration(const Stage1PassthroughFwCfg *p) {
    _cfgPtr=p;
    _cfgPtPtr=p;
    
    _cfgPtPtr->print();
  }
  
private:
  const Stage1PassthroughFwCfg *_cfgPtPtr;

  unsigned _runCall;
};

}
#endif
