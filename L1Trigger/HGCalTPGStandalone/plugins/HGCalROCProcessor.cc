#include "L1Trigger/HGCalTPGStandalone/interface/HGCalROCProcessor.h"

DEFINE_EDM_PLUGIN(HGCalVFEProcessorBaseFactory, HGCalROCProcessor, "HGCalROCProcessor");

HGCalROCProcessor::HGCalROCProcessor(const edm::ParameterSet& conf) : HGCalVFEProcessorBase(conf) {
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
  
}

void HGCalROCProcessor::initConfig(){

  std::cerr << "HGCalROCProcessor::initConfig Processing....... " << std::endl;
  typedef std::unordered_map<uint32_t, std::unordered_set<uint32_t>> trigger_map_set;
  const HGCalTriggerGeometryBase* triggerGeometry = geometry();
  std::cerr << "HGCalROCProcessor::HGCalROCProcessor triggerGeometry :  " << triggerGeometry << std::endl;

  //Collected from HGCalTriggerGeomTesterV9Imp3::fillTriggerGeometry()
  uint64_t nofdetId = 0;
  uint64_t nofdetIdCEESi = 0;
  uint64_t nofdetIdCEHSi = 0;
  uint64_t nofdetIdCEHSc = 0;
  uint64_t nofdetIdNose = 0;
  HGCalTriggerGeometryBase::geom_set modlist, modlistCEESi, modlistCEHSi, modlistCEHSc;
  HGCalTriggerGeometryBase::geom_set linklist, linklistCEESi, linklistCEHSi, linklistCEHSc;
  HGCalTriggerGeometryBase::geom_set lpGBTlist, lpGBTlistCEESi, lpGBTlistCEHSi, lpGBTlistCEHSc;
  HGCalTriggerGeometryBase::geom_set stage1list, stage1listCEESi, stage1listCEHSi, stage1listCEHSc;
  std::set<int> waferlist;//, waferlistCEESi;
  std::set<std::tuple<int, int, int>> tuplelist;
  std::set<int> rhthicknesslist;
  std::set<int> rhcelltypelist;
  std::set<int> celltypelist;
  
  for (const auto& id : triggerGeometry->eeGeometry()->getValidDetIds()) {
    HGCSiliconDetId detid(id);
    if (!triggerGeometry->eeTopology().valid(id))
      continue;
    //std::cout << "\t detid: " << detid.rawId() << std::endl;
    nofdetIdCEESi++;
    nofdetId++;
    unsigned cellId = detid.rawId();
    unsigned modId = triggerGeometry->getModuleFromCell(cellId);
    unsigned linkId = triggerGeometry->getLinksInModule(modId);
    HGCalTriggerGeometryBase::geom_set lpGBTIds = triggerGeometry->getLpgbtsFromModule(modId);
    unsigned stage1Id = 1024;
    for (auto& lpGBTId : lpGBTIds)
      stage1Id = triggerGeometry->getStage1FpgaFromLpgbt(lpGBTId);
    //unsigned stage1Id1 = triggerGeometry->getStage1FpgaFromModule(modId);
    modlist.insert(modId);
    modlistCEESi.insert(modId);
    linklist.insert(linkId);
    linklistCEESi.insert(linkId);
    lpGBTlist.insert(lpGBTIds.begin(),lpGBTIds.end());
    lpGBTlistCEESi.insert(lpGBTIds.begin(),lpGBTIds.end());
    stage1list.insert(stage1Id);
    stage1listCEESi.insert(stage1Id);
    
    //HGCalDetId hid(detid);
    //waferlist.insert(hid.waferType());
    
    //Follow methods of Geometry/HGCalCommonData/interface/HGCalDDDConstants.h
    //Follow methods of Geometry/HGCalCommonData/interface/HGCalTypes.h
    std::tuple<int, int, int> wtype = triggerGeometry->eeTopology().dddConstants().waferType(detid,true);
    waferlist.insert(get<1>(wtype));
    tuplelist.insert(wtype);

    int cellLayer = detid.layer();
    int cellWaferU = detid.waferU();
    int cellWaferV = detid.waferV();
    int sithickness = triggerGeometry->eeTopology().dddConstants().cellThickness(cellLayer, cellWaferU, cellWaferV);
    rhthicknesslist.insert(sithickness);

    
    int celltype = 0;//rhtools->getCellType(id);
    rhcelltypelist.insert(celltype);
    
    // int cellSide = detid.zside();
    // int cellU = detid.cellU();
    // int cellV = detid.cellV();
    int type1 = detid.type();
    celltypelist.insert(type1);
    
    // int type2 = triggerGeometry->eeTopology().dddConstants().getTypeHex(cellLayer, cellWaferU, cellWaferV);
    // if (type1 != type2) {
    //   std::cout << "Found incompatible wafer types:\n  " << detid << "\n";
    // }
    // GlobalPoint center = triggerGeometry->eeGeometry()->getPosition(id);
    // float cellX = center.x();
    // float cellY = center.y();
    // float cellZ = center.z();
    // std::pair<double,double> xy = triggerGeometry->eeTopology().dddConstants().waferPosition(cellLayer, cellWaferU, cellWaferV, true, false);
    // int wtype(0);
    // double wt(0);
    // triggerGeometry->eeTopology().dddConstants().waferFromPosition(HGCalParameters::k_ScaleToDDD * xy.first,
    // 								   HGCalParameters::k_ScaleToDDD * xy.second,
    // 								   cellSide,
    // 								   cellLayer,
    // 								   cellWaferU,
    // 								   cellWaferV,
    // 								   cellU,
    // 								   cellV,
    // 								   wtype,
    // 								   wt,
    // 								   true,
    // 								   false);
    // //waferlist.insert(wtype);

    // int wtype = triggerGeometry->eeTopology().dddConstants().waferType(cellLayer, cellWaferU, cellWaferV,false);
    // waferlist.insert(wtype);
    //   // cellId_ = detid.rawId();
  //   // cellSide_ = detid.zside();
  //   // cellSubdet_ = detid.subdet();
  //   // cellLayer_ = detid.layer();
  //   // cellWaferU_ = detid.waferU();
  //   // cellWaferV_ = detid.waferV();
  //   // cellU_ = detid.cellU();
  //   // cellV_ = detid.cellV();
  //   // int type1 = detid.type();
  //   // int type2 = triggerGeometry->eeTopology().dddConstants().getTypeHex(cellLayer_, cellWaferU_, cellWaferV_);
  //   // if (type1 != type2) {
  //   //   std::cout << "Found incompatible wafer types:\n  " << detid << "\n";
  //   // }
  
  //   // //
  //   // GlobalPoint center = triggerGeometry->eeGeometry()->getPosition(id);
  //   // cellX_ = center.x();
  //   // cellY_ = center.y();
  //   // cellZ_ = center.z();
  //   // cellEta_ = center.eta();
  //   // cellPhi_ = center.phi();
  //   // std::vector<GlobalPoint> corners = triggerGeometry->eeGeometry()->getCorners(id);
  //   // cellCornersN_ = corners.size();
  //   // setTreeCellCornerSize(cellCornersN_);
  //   // for (unsigned i = 0; i < corners.size(); i++) {
  //   //   cellCornersX_.get()[i] = corners[i].x();
  //   //   cellCornersY_.get()[i] = corners[i].y();
  //   //   cellCornersZ_.get()[i] = corners[i].z();
  //   // }
  //   // treeCells_->Fill();
  //   // // fill trigger cells
  //   // if (!no_trigger_) {
  //   //   uint32_t trigger_cell = triggerGeometry->getTriggerCellFromCell(id);
  //   //   // Skip trigger cells in module 0
  //   //   // uint32_t module = triggerGeometry->getModuleFromTriggerCell(trigger_cell);
  //   //   //if (HGCalDetId(module).wafer() == 0)
  //   //   //  continue;
  //   //   auto itr_insert = trigger_cells.emplace(trigger_cell, std::unordered_set<uint32_t>());
  //   //   itr_insert.first->second.emplace(id);
  //   // }
  }
  
  for (const auto& id : triggerGeometry->hsiGeometry()->getValidDetIds()) {
    HGCSiliconDetId detid(id);
    if (!triggerGeometry->hsiTopology().valid(id))
      continue;
    nofdetIdCEHSi++;
    nofdetId++;
    unsigned cellId = detid.rawId();
    unsigned modId = triggerGeometry->getModuleFromCell(cellId);
    //unsigned linkId = triggerGeometry->getLinksInModule(modId);
    HGCalTriggerGeometryBase::geom_set lpGBTIds = triggerGeometry->getLpgbtsFromModule(modId);
    unsigned stage1Id = 1024;
    for (auto& lpGBTId : lpGBTIds)
      stage1Id = triggerGeometry->getStage1FpgaFromLpgbt(lpGBTId);
    modlist.insert(modId);
    modlistCEHSi.insert(modId);
    // linklist.insert(linkId);
    // linklistCEHSi.insert(linkId);
    lpGBTlist.insert(lpGBTIds.begin(),lpGBTIds.end());
    lpGBTlistCEHSi.insert(lpGBTIds.begin(),lpGBTIds.end());
    stage1list.insert(stage1Id);
    stage1listCEHSi.insert(stage1Id);    

    // std::tuple<int, int, int> wtype = triggerGeometry->eeTopology().dddConstants().waferType(detid,false);
    // waferlist.insert(get<1>(wtype));

  }
  
  for (const auto& id : triggerGeometry->hscGeometry()->getValidDetIds()) {
    HGCScintillatorDetId detid(id);
    nofdetIdCEHSc++;
    nofdetId++;
    unsigned cellId = detid.rawId();
    unsigned modId = triggerGeometry->getModuleFromCell(cellId);
    // unsigned linkId = triggerGeometry->getLinksInModule(modId);
    HGCalTriggerGeometryBase::geom_set lpGBTIds = triggerGeometry->getLpgbtsFromModule(modId);
    unsigned stage1Id = 1024;
    for (auto& lpGBTId : lpGBTIds)
      stage1Id = triggerGeometry->getStage1FpgaFromLpgbt(lpGBTId);
    modlist.insert(modId);
    modlistCEHSc.insert(modId);
    // linklist.insert(linkId);
    // linklistCEHSc.insert(linkId);

    lpGBTlist.insert(lpGBTIds.begin(),lpGBTIds.end());
    lpGBTlistCEHSc.insert(lpGBTIds.begin(),lpGBTIds.end());
    stage1list.insert(stage1Id);
    stage1listCEHSc.insert(stage1Id);
  }

  if (triggerGeometry->isWithNoseGeometry()) {
    for (const auto& id : triggerGeometry->noseGeometry()->getValidDetIds()) {
      HFNoseDetId detid(id);
      nofdetIdNose++;
      nofdetId++;
    }
  }

  
  std::cout << "\t Nof detids (CEE-Si): " << nofdetIdCEESi
	    << ", (CEH-Si): " << nofdetIdCEHSi
	    << ", (CEH-Sc): " << nofdetIdCEHSc
	    << ", (Nose): " << nofdetIdNose
	    << std::endl;
  std::cout << "\t Nof detids: " << nofdetId << std::endl;
  std::cout << "\t Nof modules (CEE-Si): " <<  modlistCEESi.size()
	    << ", (CEH-Si): " << modlistCEHSi.size()
	    << ", (CEH-Sc): " << modlistCEHSc.size()
	    // << ", (Nose): " << nofdetIdNose
	    << std::endl;
  std::cout << "\t Nof modules : " << modlist.size() << std::endl;
  std::cout << "\t Nof links (CEE-Si): " <<  linklistCEESi.size()
	    // << ", (CEH-Si): " << linklistCEHSi.size()
	    // << ", (CEH-Sc): " << linklistCEHSc.size()
	    // << ", (Nose): " << nofdetIdNose
	    << std::endl;
  std::cout << "\t Nof links : " << linklist.size() << std::endl;
  std::cout << "\t Nof lpGBPTs (CEE-Si): " <<  lpGBTlistCEESi.size()
	    << ", (CEH-Si): " << lpGBTlistCEHSi.size()
	    << ", (CEH-Sc): " << lpGBTlistCEHSc.size()
	    // << ", (Nose): " << nofdetIdNose
	    << std::endl;
  std::cout << "\t Nof lpGBPTs : " << lpGBTlist.size() << std::endl;
  std::cout << "\t Nof Stage1FPGA (CEE-Si): " <<  stage1listCEESi.size()
	    << ", (CEH-Si): " << stage1listCEHSi.size()
	    << ", (CEH-Sc): " << stage1listCEHSc.size()
	    // << ", (Nose): " << nofdetIdNose
	    << std::endl;
  std::cout << "\t Nof Stage1FPGA : " << stage1list.size() << std::endl;
  
  
  
  // for (auto& stage1Id : stage1list) std::cout << "stage1Id: " << stage1Id << std::endl;
  // for (auto& lpGBTId : lpGBTlist) std::cout << "lpGBTId: " << lpGBTId << std::endl;
  //for (auto& modId : modlist) std::cout << "modId: " << modId << std::endl;
  for (auto& wafertype : waferlist) std::cout << "wafertype: " << wafertype << std::endl;
  for (auto& thickness : rhthicknesslist) std::cout << "RH thickness: " << thickness << std::endl;
  for (auto& rhctype : rhcelltypelist) std::cout << "RH celltype: " << rhctype << std::endl;
  for (auto& ctype : celltypelist) std::cout << "celltype: " << ctype << std::endl;

  for (auto& ituple : tuplelist) std::cout << "ituple: (" << get<0>(ituple)
					   <<", " << get<1>(ituple)
					   <<", " << get<2>(ituple)
					   << ")" << std::endl;
}

void HGCalROCProcessor::run(const HGCalDigiCollection& digiColl,
                                l1t::HGCalTriggerCellBxCollection& triggerCellColl) {
  vfeSummationImpl_->setGeometry(geometry());
  calibrationEE_->setGeometry(geometry());
  calibrationHEsi_->setGeometry(geometry());
  calibrationHEsc_->setGeometry(geometry());
  calibrationNose_->setGeometry(geometry());
  triggerTools_.setGeometry(geometry());
  
  std::cerr << "HGCalROCProcessor::run Running here....... " << std::endl;
  
  std::vector<HGCalDataFrame> dataframes;
  std::vector<std::pair<DetId, uint32_t>> linearized_dataframes;
  std::unordered_map<uint32_t, uint32_t> tc_payload;
  std::unordered_map<uint32_t, std::array<uint64_t, 2>> tc_compressed_payload;

  // Remove disconnected modules and invalid cells
  for (const auto& digiData : digiColl) {
    if (!geometry()->validCell(digiData.id()))
      continue;
    uint32_t module = geometry()->getModuleFromCell(digiData.id());

    // no disconnected layer for HFNose
    if (DetId(digiData.id()).subdetId() != ForwardSubdetector::HFNose) {
      if (geometry()->disconnectedModule(module))
        continue;
    }

    dataframes.emplace_back(digiData.id());
    for (int i = 0; i < digiData.size(); i++) {
      dataframes.back().setSample(i, digiData.sample(i));
    }
  }
  if (dataframes.empty())
    return;

  constexpr int kHighDensityThickness = 0;
  bool isSilicon = triggerTools_.isSilicon(dataframes[0].id());
  bool isEM = triggerTools_.isEm(dataframes[0].id());
  bool isNose = triggerTools_.isNose(dataframes[0].id());
  int thickness = triggerTools_.thicknessIndex(dataframes[0].id());
  // Linearization of ADC and TOT values to the same LSB
  if (isSilicon) {
    vfeLinearizationSiImpl_->linearize(dataframes, linearized_dataframes);
  } else {
    vfeLinearizationScImpl_->linearize(dataframes, linearized_dataframes);
  }
  // Sum of sensor cells into trigger cells
  vfeSummationImpl_->triggerCellSums(linearized_dataframes, tc_payload);
  // Compression of trigger cell charges to a floating point format
  if (thickness == kHighDensityThickness) {
    vfeCompressionHDMImpl_->compress(tc_payload, tc_compressed_payload);
  } else {
    vfeCompressionLDMImpl_->compress(tc_payload, tc_compressed_payload);
  }
  
  // Transform map to trigger cell vector
  for (const auto& [tc_id, tc_value] : tc_payload) {
    if (tc_value > 0) {
      const auto& [tc_compressed_code, tc_compressed_value] = tc_compressed_payload[tc_id];
      
      if (tc_compressed_value > std::numeric_limits<int>::max())
        edm::LogWarning("CompressedValueDowncasting") << "Compressed value cannot fit into 32-bit word. Downcasting.";

      l1t::HGCalTriggerCell triggerCell(
          reco::LeafCandidate::LorentzVector(), static_cast<int>(tc_compressed_value), 0, 0, 0, tc_id);

      if (tc_compressed_code > std::numeric_limits<uint32_t>::max())
        edm::LogWarning("CompressedValueDowncasting") << "Compressed code cannot fit into 32-bit word. Downcasting.";

      triggerCell.setCompressedCharge(static_cast<uint32_t>(tc_compressed_code));
      triggerCell.setUncompressedCharge(tc_value);
      GlobalPoint point = geometry()->getTriggerCellPosition(tc_id);

      // 'value' is hardware, so p4 is meaningless, except for eta and phi
      math::PtEtaPhiMLorentzVector p4((double)tc_compressed_value / cosh(point.eta()), point.eta(), point.phi(), 0.);
      triggerCell.setP4(p4);
      triggerCell.setPosition(point);

      // calibration
      if (triggerCell.hwPt() > 0) {
        l1t::HGCalTriggerCell calibratedtriggercell(triggerCell);
        if (isNose) {
          calibrationNose_->calibrateInGeV(calibratedtriggercell);
        } else if (isSilicon) {
          if (isEM) {
            calibrationEE_->calibrateInGeV(calibratedtriggercell);
          } else {
            calibrationHEsi_->calibrateInGeV(calibratedtriggercell);
          }
        } else {
          calibrationHEsc_->calibrateInGeV(calibratedtriggercell);
        }
        triggerCellColl.push_back(0, calibratedtriggercell);
      }
    }
  }
}
