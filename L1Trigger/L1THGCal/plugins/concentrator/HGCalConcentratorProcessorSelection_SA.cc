#include "L1Trigger/L1THGCal/interface/HGCalProcessorBase.h"
#include "L1Trigger/L1THGCal/interface/concentrator/HGCalConcentratorTrigSumImpl.h"
#include "L1Trigger/L1THGCal/interface/concentrator_emulator/HGCalECONTEmulImpl.h"

#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"
#include "DataFormats/L1THGCal/interface/HGCalTriggerCell.h"
#include "DataFormats/L1THGCal/interface/HGCalTriggerSums.h"
#include "DataFormats/L1THGCal/interface/HGCalConcentratorData.h"
#include "DataFormats/HGCDigi/interface/HGCDigiCollections.h"

#include <utility>
#include <tuple>
#include <limits>

class HGCalConcentratorProcessorSelectionSA : public HGCalConcentratorProcessorBase {
private:
  enum SelectionType { thresholdSelect, bestChoiceSelect, superTriggerCellSelect, autoEncoderSelect, noSelection };

public:
  HGCalConcentratorProcessorSelectionSA(const edm::ParameterSet& conf);

  void run(const edm::Handle<l1t::HGCalTriggerCellBxCollection>& triggerCellCollInput,
           std::tuple<l1t::HGCalTriggerCellBxCollection,
                      l1t::HGCalTriggerSumsBxCollection,
                      l1t::HGCalConcentratorDataBxCollection>& triggerCollOutput) override;

private:
  static constexpr int kHighDensityThickness_ = 0;
  static constexpr int kNSubDetectors_ = 3;

  std::vector<SelectionType> selectionType_;

  std::unique_ptr<HGCalConcentratorTrigSumImpl> trigSumImpl_;
  std::unique_ptr<HGCalECONTEmulImpl> econtemulImpl_;

  HGCalTriggerTools triggerTools_;
};

HGCalConcentratorProcessorSelectionSA::HGCalConcentratorProcessorSelectionSA(const edm::ParameterSet& conf)
  : HGCalConcentratorProcessorBase(conf),
    selectionType_(kNSubDetectors_) 
{
  std::vector<std::string> selectionType(conf.getParameter<std::vector<std::string>>("Method"));
  if (selectionType.size() != kNSubDetectors_ ) {
    throw cms::Exception("HGCTriggerParameterError")
      << "Inconsistent number of sub-detectors (should be " << kNSubDetectors_ << ")";
  }

  for (int subdet = 0; subdet < kNSubDetectors_; subdet++) {
    if (selectionType[subdet] == "thresholdSelect") {
      selectionType_[subdet] = thresholdSelect;
      // if (!thresholdImpl_)
      //   thresholdImpl_ = std::make_unique<HGCalConcentratorThresholdImpl>(conf);
      if (!trigSumImpl_)
        trigSumImpl_ = std::make_unique<HGCalConcentratorTrigSumImpl>(conf);
      if(!econtemulImpl_)
	econtemulImpl_ = std::make_unique<HGCalECONTEmulImpl>(conf);
    }
  }
}

void HGCalConcentratorProcessorSelectionSA::run(const edm::Handle<l1t::HGCalTriggerCellBxCollection>& triggerCellCollInput,
                                              std::tuple<l1t::HGCalTriggerCellBxCollection,
                                                         l1t::HGCalTriggerSumsBxCollection,
                                                         l1t::HGCalConcentratorDataBxCollection>& triggerCollOutput) {
  if (trigSumImpl_)
    trigSumImpl_->setGeometry(geometry());
  if (econtemulImpl_)
    econtemulImpl_->setGeometry(geometry());
  triggerTools_.setGeometry(geometry());

  auto& triggerCellCollOutput = std::get<0>(triggerCollOutput);
  auto& triggerSumCollOutput = std::get<1>(triggerCollOutput);
  auto& autoEncoderCollOutput = std::get<2>(triggerCollOutput);

  const l1t::HGCalTriggerCellBxCollection& collInput = *triggerCellCollInput;

  std::unordered_map<uint32_t, std::vector<l1t::HGCalTriggerCell>> tc_modules;
  for (const auto& trigCell : collInput) {
    uint32_t module = geometry()->getModuleFromTriggerCell(trigCell.detId());
    tc_modules[module].push_back(trigCell);
    GlobalPoint tcpos = triggerTools_.getTCPosition(trigCell.detId());
    unsigned layerId = triggerTools_.layerWithOffset(trigCell.detId());
    if(tcpos.z()>0.0)
      std::cout<< "SA: Layer : " << std::setw(2) << std::setfill('0') << layerId 
	       <<" , zside : "<<tcpos.z()
	       <<" , trigcell.detId() : "<<trigCell.detId()
	       <<" , Input : "<<trigCell.compressedCharge()
	       <<" , Module :" << module 
	       << std::endl;
  }
  
  for (const auto& module_trigcell : tc_modules) {
    std::vector<l1t::HGCalTriggerCell> trigCellVecOutput;
    std::vector<l1t::HGCalTriggerCell> trigCellVecCoarsened;
    std::vector<l1t::HGCalTriggerCell> trigCellVecNotSelected;
    std::vector<l1t::HGCalTriggerSums> trigSumsVecOutput;
    std::vector<l1t::HGCalConcentratorData> ae_EncodedLayerOutput;

    int thickness = triggerTools_.thicknessIndex(module_trigcell.second.at(0).detId());

    HGCalTriggerTools::SubDetectorType subdet = triggerTools_.getSubDetectorType(module_trigcell.second.at(0).detId());
    int nofTrigCells = 0;

    econtemulImpl_->select(module_trigcell.second, trigCellVecOutput, trigCellVecNotSelected, nofTrigCells);
    
    // trigger sum
    if (trigSumImpl_) {
      trigSumImpl_->doSum(module_trigcell.first, module_trigcell.second, trigSumsVecOutput);
    }

    for (const auto& trigCell : trigCellVecOutput) {
      triggerCellCollOutput.push_back(0, trigCell);
    }
    for (const auto& trigSums : trigSumsVecOutput) {
      triggerSumCollOutput.push_back(0, trigSums);
    }
    for (const auto& aeVal : ae_EncodedLayerOutput) {
      autoEncoderCollOutput.push_back(0, aeVal);
    }
    
  }
}

DEFINE_EDM_PLUGIN(HGCalConcentratorFactory, HGCalConcentratorProcessorSelectionSA, "HGCalConcentratorProcessorSelectionSA");
