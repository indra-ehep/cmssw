#ifndef hgcal_roc_DataChannelTC_h
#define hgcal_roc_DataChannelTC_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <cassert>

#include "ConfigurationROC.h"

namespace hgcal_roc{
  
  class DataChannelTC {
  public:
    DataChannelTC(ConfigurationChannelROC &cc) : cfgCh_(cc) {}
    
    uint32_t compress(uint32_t val) {
      
      val = (cfgCh_.ldm())?val>>1:val>>3;
      if(val>cfgCh_.tccomp_max()) val = cfgCh_.tccomp_max();
      
      uint32_t expo ;
      uint32_t mant ;
      
      if(val>7){
	uint32_t r = 0; 
	uint32_t v = val; 
	r = 0; 
	while (v >>= 1) r++;
	expo = r - 2;
	uint32_t shift = r - 3;
	mant = (val>>shift)&0x7;
      }else{
	expo = 0;
	mant = val&0x7;
      }
      
      return ((expo<<3) | mant);
    }
    
    uint32_t decoding(uint32_t compressed)
    {
      uint32_t mant = compressed & 0x7;
      uint32_t expo = (compressed>>3) & 0xF;

      if(expo==0) 
	return (cfgCh_.ldm()) ? (mant<<1) : (mant<<3) ;

      uint32_t shift = expo+2;
      uint32_t decomp = 1<<shift;
      decomp = decomp | (mant<<(shift-3));
      decomp = (cfgCh_.ldm()) ? (decomp<<1) : (decomp<<3) ;

      return decomp;
    }

    void print() const {
      std::cout << "DataChannelTC::print()" << std::endl;    
      cfgCh_.print();
    }
    
    ConfigurationChannelROC &cfgCh_;
  };
}

#endif
