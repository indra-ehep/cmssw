#ifndef __L1Trigger_HGCalTPGStandalone_HGCalROCProcessor_h__
#define __L1Trigger_HGCalTPGStandalone_HGCalROCProcessor_h__

#include "L1Trigger/L1THGCal/interface/HGCalProcessorBase.h"

#include "L1Trigger/L1THGCal/interface/veryfrontend/HGCalVFELinearizationImpl.h"
#include "L1Trigger/L1THGCal/interface/veryfrontend/HGCalVFESummationImpl.h"
#include "L1Trigger/L1THGCal/interface/HGCalVFECompressionImpl.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerCellCalibration.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"

#include "TPGFEDataformat.hh"
#include "TPGBEDataformat.hh"
#include "TPGFEEmulation/TPGFEConfiguration.hh"
#include "TPGFEEmulation/TPGFEModuleEmulation.hh"

#include "L1Trigger/L1THGCal/interface/HGCalTriggerGeometryBase.h"

class HGCalROCProcessor : public HGCalVFEProcessorBase {
public:
  HGCalROCProcessor(const edm::ParameterSet& conf);

  void run(const HGCalDigiCollection& digiColl, l1t::HGCalTriggerCellBxCollection& triggerCellColl) override;
  
  void initConfig() override;
  
private:
  std::unique_ptr<HGCalVFELinearizationImpl> vfeLinearizationSiImpl_;
  std::unique_ptr<HGCalVFELinearizationImpl> vfeLinearizationScImpl_;
  std::unique_ptr<HGCalVFESummationImpl> vfeSummationImpl_;
  std::unique_ptr<HGCalVFECompressionImpl> vfeCompressionLDMImpl_;
  std::unique_ptr<HGCalVFECompressionImpl> vfeCompressionHDMImpl_;
  std::unique_ptr<HGCalTriggerCellCalibration> calibrationEE_;
  std::unique_ptr<HGCalTriggerCellCalibration> calibrationHEsi_;
  std::unique_ptr<HGCalTriggerCellCalibration> calibrationHEsc_;
  std::unique_ptr<HGCalTriggerCellCalibration> calibrationNose_;

  HGCalTriggerTools triggerTools_;
};

#endif
