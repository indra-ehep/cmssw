#include "L1Trigger/L1THGCal/interface/econt_emulator/HGCalECONTEmulImpl.h"


HGCalECONTEmulImpl::HGCalECONTEmulImpl(const edm::ParameterSet& conf)
    : threshold_silicon_(conf.getParameter<double>("threshold_silicon")),
      threshold_scintillator_(conf.getParameter<double>("threshold_scintillator")) {



  configCh.ldm(false);
  configCh.dropLsb(2);
  configCh.quadSetting(1);
  configCh.calibration(0x800);
  configCh.print();
  
}

void HGCalECONTEmulImpl::select(const std::vector<l1t::HGCalTriggerCell>& trigCellVecInput,
                                            std::vector<l1t::HGCalTriggerCell>& trigCellVecOutput,
                                            std::vector<l1t::HGCalTriggerCell>& trigCellVecNotSelected) {
  constexpr int kHighDensityThickness = 0;
  for (const auto& trigCell : trigCellVecInput) {
    bool isScintillator = triggerTools_.isScintillator(trigCell.detId());
    int thickness = triggerTools_.thicknessIndex(trigCell.detId());
    bool moduleDensity = (thickness!=kHighDensityThickness) ? true : false;
    configCh.ldm(moduleDensity);    
    hgcal_econt::DataChannel data(configCh);
    data.setInputCodeTo(uint8_t(trigCell.compressedCharge()));
    l1t::HGCalTriggerCell outTrigCell(trigCell);
    outTrigCell.setCompressedCharge(uint32_t(data.outputCode()));
    
    double threshold = (isScintillator ? threshold_scintillator_ : threshold_silicon_);
    if (trigCell.mipPt() >= threshold) {
      trigCellVecOutput.push_back(outTrigCell);
    } else {
      trigCellVecNotSelected.push_back(outTrigCell);
    }
  }
}
