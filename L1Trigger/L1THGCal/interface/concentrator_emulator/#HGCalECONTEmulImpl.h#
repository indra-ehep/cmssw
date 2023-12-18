#ifndef __L1Trigger_L1THGCal_HGCalECONTEmulImpl_h__
#define __L1Trigger_L1THGCal_HGCalECONTEmulImpl_h__

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/L1THGCal/interface/HGCalTriggerCell.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"
#include "L1Trigger/L1THGCal/interface/concentrator_emulator/standalone/DataChannel.h"

#include <vector>

class HGCalECONTEmulImpl {
public:
  HGCalECONTEmulImpl(const edm::ParameterSet& conf);

  void select(const std::vector<l1t::HGCalTriggerCell>& trigCellVecInput,
              std::vector<l1t::HGCalTriggerCell>& trigCellVecOutput,
		std::vector<l1t::HGCalTriggerCell>& trigCellVecNotSelected,
		int& nofTrigCells);
  
  void setGeometry(const HGCalTriggerGeometryBase* const geom) { triggerTools_.setGeometry(geom); }
  
private:
  double threshold_silicon_;
  double threshold_scintillator_;

  HGCalTriggerTools triggerTools_;

  hgcal_econt::ConfigurationChannel configCh;
};

#endif
