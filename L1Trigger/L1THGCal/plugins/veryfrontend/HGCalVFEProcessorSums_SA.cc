#include "L1Trigger/L1THGCal/interface/HGCalProcessorBase.h"

#include "L1Trigger/L1THGCal/interface/veryfrontend/HGCalVFELinearizationImpl.h"
#include "L1Trigger/L1THGCal/interface/veryfrontend/HGCalVFESummationImpl.h"
#include "L1Trigger/L1THGCal/interface/HGCalVFECompressionImpl.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerCellCalibration.h"
#include "L1Trigger/L1THGCal/interface/HGCalTriggerTools.h"
#include "L1Trigger/L1THGCal/interface/veryfrontend_emulator/HGCalROCEmulImpl.h"

class HGCalVFEProcessorSumsSA : public HGCalVFEProcessorBase {
public:
  HGCalVFEProcessorSumsSA(const edm::ParameterSet& conf);

  void run(const HGCalDigiCollection& digiColl, l1t::HGCalTriggerCellBxCollection& triggerCellColl) override;

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
  std::unique_ptr<HGCalROCEmulImpl> rocImpl_;

  HGCalTriggerTools triggerTools_;
};


HGCalVFEProcessorSumsSA::HGCalVFEProcessorSumsSA(const edm::ParameterSet& conf) : HGCalVFEProcessorBase(conf) {
  vfeLinearizationSiImpl_ =
      std::make_unique<HGCalVFELinearizationImpl>(conf.getParameter<edm::ParameterSet>("linearizationCfg_si"));
  vfeLinearizationScImpl_ =
      std::make_unique<HGCalVFELinearizationImpl>(conf.getParameter<edm::ParameterSet>("linearizationCfg_sc"));

  vfeSummationImpl_ = std::make_unique<HGCalVFESummationImpl>(conf.getParameter<edm::ParameterSet>("summationCfg"));

  vfeCompressionLDMImpl_ =
      std::make_unique<HGCalVFECompressionImpl>(conf.getParameter<edm::ParameterSet>("compressionCfg_ldm"));
  vfeCompressionHDMImpl_ =
      std::make_unique<HGCalVFECompressionImpl>(conf.getParameter<edm::ParameterSet>("compressionCfg_hdm"));

  calibrationEE_ =
      std::make_unique<HGCalTriggerCellCalibration>(conf.getParameter<edm::ParameterSet>("calibrationCfg_ee"));
  calibrationHEsi_ =
      std::make_unique<HGCalTriggerCellCalibration>(conf.getParameter<edm::ParameterSet>("calibrationCfg_hesi"));
  calibrationHEsc_ =
      std::make_unique<HGCalTriggerCellCalibration>(conf.getParameter<edm::ParameterSet>("calibrationCfg_hesc"));
  calibrationNose_ =
      std::make_unique<HGCalTriggerCellCalibration>(conf.getParameter<edm::ParameterSet>("calibrationCfg_nose"));

  rocImpl_ = std::make_unique<HGCalROCEmulImpl>(conf);
}

void HGCalVFEProcessorSumsSA::run(const HGCalDigiCollection& digiColl,
                                l1t::HGCalTriggerCellBxCollection& triggerCellColl) {
  vfeSummationImpl_->setGeometry(geometry());
  calibrationEE_->setGeometry(geometry());
  calibrationHEsi_->setGeometry(geometry());
  calibrationHEsc_->setGeometry(geometry());
  calibrationNose_->setGeometry(geometry());
  triggerTools_.setGeometry(geometry());
  rocImpl_->setGeometry(geometry());

  std::vector<HGCalDataFrame> dataframes;
  std::vector<std::pair<DetId, uint32_t>> linearized_dataframes;
  std::unordered_map<uint32_t, uint32_t> tc_payload;
  std::unordered_map<uint32_t, std::array<uint64_t, 2>> tc_compressed_payload;

  // Remove disconnected modules and invalid cells
  for (const auto& digiData : digiColl) {
    unsigned layerId = triggerTools_.layerWithOffset(digiData.id());
    int zside = triggerTools_.zside(digiData.id());
    if (!geometry()->validCell(digiData.id())){
      if(zside>0.)
	std::cout<<"HGCalVFEProcessorSumsSA::run:digiloop "<<", layerId : "<<layerId
		 <<" , DetID : "<<uint32_t(digiData.id())<<", skipping validcell"<<std::endl; 
      continue;
    }
    uint32_t module = geometry()->getModuleFromCell(digiData.id());

    // no disconnected layer for HFNose
    if (DetId(digiData.id()).subdetId() != ForwardSubdetector::HFNose) {
      if (geometry()->disconnectedModule(module)){
	if(zside>0.){
	  std::cout<<"HGCalVFEProcessorSumsSA::run:digiloop "<<", layerId : "<<layerId
		   <<" , DetID : "<<uint32_t(digiData.id())<<", skipping module"<<std::endl; 	
	}
        continue;
      }
    }
    
    
    //unsigned layerId = triggerTools_.layerWithOffset(digiData.id());
    //int zside = triggerTools_.zside(digiData.id());
    if(zside>0.)
      std::cout<<"HGCalVFEProcessorSumsSA::run:digiloop "; 
    dataframes.emplace_back(digiData.id());
    for (int i = 0; i < digiData.size(); i++) {
      dataframes.back().setSample(i, digiData.sample(i));
      if(i==2 and zside>0.)
	std::cout<<", layerId : "<<layerId<<" , DetID : "<<uint32_t(digiData.id())<<", data : "<<digiData.sample(i).data()<<std::endl;
    }
  }
  if (dataframes.empty())
    return;

  constexpr int kHighDensityThickness = 0;
  bool isSilicon = triggerTools_.isSilicon(dataframes[0].id());
  // bool isEM = triggerTools_.isEm(dataframes[0].id());
  // bool isNose = triggerTools_.isNose(dataframes[0].id());
  int thickness = triggerTools_.thicknessIndex(dataframes[0].id());
  // Linearization of ADC and TOT values to the same LSB
  if (isSilicon) {
    vfeLinearizationSiImpl_->linearize(dataframes, linearized_dataframes);
  } else {
    vfeLinearizationScImpl_->linearize(dataframes, linearized_dataframes);
  }
  // for (const auto& frame : dataframes) {
  //   for (const auto& linframe : linearized_dataframes){
  //     if(frame.id()==linframe.first){
  // 	unsigned layerId = triggerTools_.layerWithOffset(frame.id());
  // 	int zside = triggerTools_.zside(frame.id());
  // 	if(zside>0.)
  // 	  std::cout<<"HGCalVFEProcessorSumsSA::run:linearized layerId : "<<layerId<<", DetID : "<<uint32_t(frame.id())<<", data : "<<frame[2].data() <<", mode : "<<frame[2].mode()<<", amplitude_int : "<< linframe.second <<std::endl;
  //     }
  //   }
  // }
  
  // Sum of sensor cells into trigger cells
  vfeSummationImpl_->triggerCellSums(linearized_dataframes, tc_payload);
  // for (const auto& linframe : linearized_dataframes){
  //   uint32_t tcid = triggerTools_.getTriggerGeometry ()-> getTriggerCellFromCell( linframe.first );
  //     for (const auto& tc : tc_payload){
  //     if(tc.first==tcid){
  // 	unsigned layerId = triggerTools_.layerWithOffset(linframe.first);
  // 	int zside = triggerTools_.zside(linframe.first);
  // 	if(zside>0.)
  // 	  std::cout<<"HGCalVFEProcessorSumsSA::run:cellsum layerId : "<<layerId<<", DetID : "<<uint32_t(linframe.first)<<", tc_id : "<<tc.first<<", amplitude_int : "<< linframe.second <<", summed amplitude : " << tc.second <<std::endl;
  //     }
  //   }
  // }
  
  
  // Compression of trigger cell charges to a floating point format
  if (thickness == kHighDensityThickness) {
    vfeCompressionHDMImpl_->compress(tc_payload, tc_compressed_payload);
  } else {
    vfeCompressionLDMImpl_->compress(tc_payload, tc_compressed_payload);
  }
  for (const auto& tc : tc_payload){
    for (const auto& tc_comp : tc_compressed_payload){
      if(tc.first==tc_comp.first){
  	unsigned layerId = triggerTools_.layerWithOffset(tc.first);
  	int zside = triggerTools_.zside(tc.first);
  	if(zside>0.)
  	  std::cout<<"HGCalVFEProcessorSumsSA::run:compres layerId : "<<layerId<<", tc_id : "<<tc.first<<", thickness : "<<thickness<<", summed amplitude : " << tc.second <<", comp_f : " << tc_comp.second[0]<<", comp_s : " << tc_comp.second[1] <<std::endl;
      }
    }
  }
  
  rocImpl_->process(dataframes, triggerCellColl);
  // for( const auto& tc : triggerCellColl){
  //   std::cout<<"HGCalVFEProcessorSumsSA::run:tcid : "<<tc.detId()<<", uncompressed : "<<tc.uncompressedCharge()<<", compressed : "<<tc.compressedCharge() << std::endl;
  // }
  
  // // Transform map to trigger cell vector
  // for (const auto& [tc_id, tc_value] : tc_payload) {
  //   if (tc_value > 0) {
  //     const auto& [tc_compressed_code, tc_compressed_value] = tc_compressed_payload[tc_id];

  //     if (tc_compressed_value > std::numeric_limits<int>::max())
  //       edm::LogWarning("CompressedValueDowncasting") << "Compressed value cannot fit into 32-bit word. Downcasting.";

  //     l1t::HGCalTriggerCell triggerCell(
  //         reco::LeafCandidate::LorentzVector(), static_cast<int>(tc_compressed_value), 0, 0, 0, tc_id);

  //     if (tc_compressed_code > std::numeric_limits<uint32_t>::max())
  //       edm::LogWarning("CompressedValueDowncasting") << "Compressed code cannot fit into 32-bit word. Downcasting.";

  //     triggerCell.setCompressedCharge(static_cast<uint32_t>(tc_compressed_code));
  //     triggerCell.setUncompressedCharge(tc_value);
  //     GlobalPoint point = geometry()->getTriggerCellPosition(tc_id);

  //     // 'value' is hardware, so p4 is meaningless, except for eta and phi
  //     math::PtEtaPhiMLorentzVector p4((double)tc_compressed_value / cosh(point.eta()), point.eta(), point.phi(), 0.);
  //     triggerCell.setP4(p4);
  //     triggerCell.setPosition(point);

  //     // calibration
  //     if (triggerCell.hwPt() > 0) {
  //       l1t::HGCalTriggerCell calibratedtriggercell(triggerCell);
  //       if (isNose) {
  //         calibrationNose_->calibrateInGeV(calibratedtriggercell);
  //       } else if (isSilicon) {
  //         if (isEM) {
  //           calibrationEE_->calibrateInGeV(calibratedtriggercell);
  //         } else {
  //           calibrationHEsi_->calibrateInGeV(calibratedtriggercell);
  //         }
  //       } else {
  //         calibrationHEsc_->calibrateInGeV(calibratedtriggercell);
  //       }
  //       triggerCellColl.push_back(0, calibratedtriggercell);
  //     }
  //   }
  // }

}

DEFINE_EDM_PLUGIN(HGCalVFEProcessorBaseFactory, HGCalVFEProcessorSumsSA, "HGCalVFEProcessorSumsSA");
