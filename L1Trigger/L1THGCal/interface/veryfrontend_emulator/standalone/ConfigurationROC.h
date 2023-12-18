#ifndef hgcal_roc_ConfigurationROC_h
#define hgcal_roc_ConfigurationROC_h

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <cassert>

namespace hgcal_roc{
  
  class ConfigurationChannelROC {
  public:
    ConfigurationChannelROC() {}
    
    ////Settings
    ///////////////////////////////////
    void channel(unsigned ch) {
      channel_ = ch ;
    }
    void data(uint16_t chdata) {
      data_ = chdata ;
    }
    void mode(bool totmode) {
      mode_ = totmode ;
    }
    void ldm(bool isldm) {
      ldm_ = isldm ;
    }
    void sci(bool issci) {
      sci_ = issci ;
    }
    void pedestal_adc(uint16_t pedestal) {
      pedestal_adc_ = pedestal;
    }
    void threshold_adc(uint16_t thresh) {
      threshold_adc_ = thresh;
    }
    void pedestal_tot(uint16_t pedestal) {
      pedestal_tot_ = pedestal;
    }
    void plateau(uint16_t plateau) {
      plateau_tot_ = plateau;
    }
    void multfactor(uint16_t mult) {
      multfact_tot_ = mult;
    }
    void dropLsb(unsigned lsbbit) {
      dropLsb_ = lsbbit;
    }
    void tdc_onset_si(uint16_t tdconset) {
      tdc_onset_si_ = tdconset;
    }
    void tdc_onset_sci(uint16_t tdconset) {
      tdc_onset_sci_ = tdconset;
    }
    
    ////////////////////////////////////

    ////Get Values
    ///////////////////////////////////
    unsigned channel() const {
      return channel_ ;
    }
    uint16_t data() const {
      return data_ ;
    }
    bool mode() const {
      return mode_ ;
    }
    bool ldm() const {
      return ldm_ ;
    }
    bool sci() const {
      return sci_ ;
    }
    uint16_t pedestal_adc() const {
      return pedestal_adc_ ;
    }
    uint16_t threshold_adc() const {
      return threshold_adc_ ;
    }
    uint16_t pedestal_tot() const {
      return pedestal_tot_ ;
    }
    uint16_t plateau() const {
      return plateau_tot_ ;
    }
    uint16_t multfactor() const {
      return multfact_tot_ ;
    }
    unsigned dropLsb() const {
      return dropLsb_;
    }

    const uint32_t lindata_max() const {
      return lindata_max_ ;
    }
    const uint32_t tcdata_max() const {
      return (ldm_) ? tcdata_max_ld_ : tcdata_max_hd_ ;
    }
    const uint32_t tccomp_max() const {
      return tccompress_max_ ;
    }
    uint16_t tdc_onset() const {
      return (sci_) ? tdc_onset_sci_ : tdc_onset_si_ ;
    }

    ///////////////////////////////////
    
  
    void print() const {

      std::cout << "ConfigurationChannelROC::print()  Channel "
		<< channel_ << std::endl;
      std::cout << " Module = " << (ldm_?"LDM":"HDM")
		<< ", drop LSB = " << dropLsb_ << std::endl;    
      /* std::cout << " Threshold = 0x" */
      /* 		<< std::setw(6) << threshold_adc_ << std::endl; */
      std::cout << std::dec << std::setfill(' ') << std::endl;
      
    }
    
    //private:
    
    unsigned channel_;       //channel number
    uint16_t data_;          //(10 bit)ADC or (12 bit)TOT input
    bool mode_;              //is TOT mode input
    bool ldm_;               //is low or high density modules
    bool sci_;               //is silicon or scintiallator detector
    uint16_t pedestal_adc_;  //(8 bit)ADC pedestal
    uint16_t threshold_adc_; //(5 bit)ADC threshold 
    uint16_t pedestal_tot_;  //(7 bit)TOT pedestal
    uint16_t plateau_tot_;   //(8 bit)TOT plateau
    uint16_t multfact_tot_;  //(5 bit)TOT multfactor (default=25 for 100fC ADC saturation)
    uint16_t tdc_onset_sci_; //Assuming 60fC/55fC tdc onset [0.6*(2^10-1)/0.55*(2^10-1) for Si/Sci]
    uint16_t tdc_onset_si_;  //Assuming 60fC/55fC tdc onset [0.6*(2^10-1)/0.55*(2^10-1) for Si/Sci]

    unsigned dropLsb_;
    
    // maximum bit length after linearization [12 bit of TOT times 31 (multfactor for 80 fC ADC saturation)]
    const uint32_t lindata_max_ = 0x1FFFF;  
    const uint32_t tcdata_max_ld_ = 0x7FFFF; // adding 4 sensor data i.e. TC4
    const uint32_t tcdata_max_hd_ = 0x1FFFFF; // adding 9 sensor data i.e. TC9
    const uint32_t tccompress_max_ = 0x3FFFF; // adding 9 sensor data i.e. TC9
  };
}

#endif
