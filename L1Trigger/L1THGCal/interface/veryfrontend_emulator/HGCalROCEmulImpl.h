#ifndef __L1Trigger_L1THGCal_HGCalROCEmulImpl_h__
#define __L1Trigger_L1THGCal_HGCalROCEmulImpl_h__

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/L1THGCal/interface/HGCalTriggerCell.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"
#include "L1Trigger/L1THGCal/interface/veryfrontend_emulator/standalone/DataChannelROC.h"
#include "L1Trigger/L1THGCal/interface/veryfrontend_emulator/standalone/DataChannelTC.h"
#include "DataFormats/HGCDigi/interface/HGCDigiCollections.h"
#include <vector>
#include <TMath.h>

class HGCalROCEmulImpl {
public:
  HGCalROCEmulImpl(const edm::ParameterSet& conf);
  
  void process(const std::vector<HGCalDataFrame>& dataframes, l1t::HGCalTriggerCellBxCollection& triggerCellColl);  
  void setGeometry(const HGCalTriggerGeometryBase* const geom) { triggerTools_.setGeometry(geom); }
  
private:
  uint16_t threshold_silicon_[3]; //for different thicknesses 
  uint16_t threshold_scintillator_; 

  HGCalTriggerTools triggerTools_;

  hgcal_roc::ConfigurationChannelROC configCh;
};

#endif
