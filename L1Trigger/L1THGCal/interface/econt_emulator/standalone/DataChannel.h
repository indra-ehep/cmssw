#ifndef hgcal_econt_DataChannel_h
#define hgcal_econt_DataChannel_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <cassert>

#include "Configuration.h"

namespace hgcal_econt{
  
  class DataChannel {
  public:
    DataChannel(const ConfigurationChannel &cc) : cfgCh_(cc) {}

    void setInputCodeTo(uint8_t ic) {
      inputCode_=ic;

      uint32_t e(inputCode_>>3);
      uint32_t m(inputCode_&0x7);

      if(cfgCh_.ldm()) {
	rawEnergy_=2*m+1;
	if(e>=1) rawEnergy_+=2*8;
	if(e>=2) rawEnergy_=rawEnergy_<<(e-1);
      } else {
	rawEnergy_=8*m+4;
	if(e>=1) rawEnergy_+=8*8;
	if(e>=2) rawEnergy_=rawEnergy_<<(e-1);
      }//check ldm()

      calEnergy_=(uint64_t(rawEnergy_)*cfgCh_.calibration())>>11;

      present_=calEnergy_>=cfgCh_.threshold();
    
      outEnergy_=calEnergy_>>cfgCh_.dropLsb();

      if(outEnergy_<16) outputCode_=outEnergy_;
      else {
	for(unsigned i(0);i<32;i++) {
	  if((outEnergy_&(1<<(31-i)))!=0) {
	    e=29-i;
	    m=(outEnergy_>>(e-1))&0x7;
	  
	    //std::cout << "Temp i,e,m = " << i << ", " << e << ", " << m << std::endl;
	  
	    outputCode_=8*e+m;
	    if(outputCode_>127) outputCode_=127;
	    i=999;
	  }
	}
      }//Check outEnergy_

      e=(outputCode_>>3);
      m=(outputCode_&0x7);
    
      outputDecode_=m;
      if(e>=1) outputDecode_+=8;
      if(e>=2) outputDecode_=outputDecode_<<(e-1);
    }//closing SetInputCodeTo()

    uint8_t outputCode() const {
      return outputCode_;
    }

    void print() const {
      std::cout << "DataChannel::print()" << std::endl;    
      cfgCh_.print();
      std::cout << " Input code = " << uint16_t(inputCode_)  << std::endl;
      std::cout << " Raw energy = " << rawEnergy_ << std::endl;
      std::cout << " Cal energy = " << calEnergy_
		<< " (threshold = " << cfgCh_.threshold() << ")" << std::endl;
      std::cout << " Out energy = " << outEnergy_ << std::endl;
      std::cout << " Output code = " << uint16_t(outputCode_) << std::endl;
      std::cout << " Output decode = " << outputDecode_ << std::endl;
      std::cout << " " << (present_?"Present":"Absent") << std::endl;
    }
  
    const ConfigurationChannel &cfgCh_;
    uint8_t inputCode_;
    uint32_t rawEnergy_;
    uint32_t calEnergy_;
    uint32_t outEnergy_;
    uint8_t outputCode_ = 0x0;
    uint32_t outputDecode_;
    bool present_;
  };
}

#endif
