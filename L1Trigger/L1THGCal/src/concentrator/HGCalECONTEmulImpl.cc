#include "L1Trigger/L1THGCal/interface/concentrator_emulator/HGCalECONTEmulImpl.h"


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
				std::vector<l1t::HGCalTriggerCell>& trigCellVecNotSelected,
				int& nofTrigCells
				) 
{
  constexpr int kHighDensityThickness = 0;
  //int nofTrigCells = 0;
  for (const auto& trigCell : trigCellVecInput) {
    bool isScintillator = triggerTools_.isScintillator(trigCell.detId());
    int thickness = triggerTools_.thicknessIndex(trigCell.detId());
    bool moduleDensity = (thickness!=kHighDensityThickness) ? true : false;
    configCh.ldm(moduleDensity);    
    hgcal_econt::DataChannel data(configCh);
    data.setInputCodeTo(uint8_t(trigCell.compressedCharge()));
    l1t::HGCalTriggerCell outTrigCell(trigCell);
    outTrigCell.setCompressedCharge(uint32_t(data.outputCode()));
    trigCellVecOutput.push_back(outTrigCell);
    std::cout<<"SA:HGCalECONTEmulImpl::select: Input : "<<trigCell.compressedCharge() << ", output : " << uint16_t(data.outputCode()) << std::endl;
    // double threshold = (isScintillator ? threshold_scintillator_ : threshold_silicon_);
    // if (trigCell.mipPt() >= threshold) {
    //   trigCellVecOutput.push_back(outTrigCell);
    // } else {
    //   trigCellVecNotSelected.push_back(outTrigCell);
    // }
    nofTrigCells++;
  }
  //std::cout << "Nof Trigger cells : " << nofTrigCells << std::endl;
}
