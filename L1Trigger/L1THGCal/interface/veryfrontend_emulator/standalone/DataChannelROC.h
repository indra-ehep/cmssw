#ifndef hgcal_roc_DataChannelROC_h
#define hgcal_roc_DataChannelROC_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <cassert>

#include "ConfigurationROC.h"

namespace hgcal_roc{
  
  class DataChannelROC {
  public:
    DataChannelROC(ConfigurationChannelROC &cc) : cfgCh_(cc) {}
    
    uint32_t linearize() {
      uint32_t ampint = 0;
      if(cfgCh_.mode()){//TOT mdoe
	if(cfgCh_.data()>cfgCh_.plateau())
	  ampint = cfgCh_.data() - cfgCh_.pedestal_tot();
	else
	  ampint = cfgCh_.plateau() - cfgCh_.pedestal_tot(); //plateau
	ampint *= cfgCh_.multfactor();
	ampint +=  cfgCh_.tdc_onset(); 
      }else{ //ADC mode
	if(cfgCh_.data()>cfgCh_.threshold_adc())
	  ampint = cfgCh_.data() - cfgCh_.pedestal_adc();
	else
	  ampint = 0;
      }
      if(ampint > cfgCh_.lindata_max()) ampint = cfgCh_.lindata_max();
      return ampint;
    }
    
    
    void print() const {
      std::cout << "DataChannelROC::print()" << std::endl;    
      cfgCh_.print();
    }
  
    ConfigurationChannelROC &cfgCh_;
  };
}

#endif
