#include "L1Trigger/L1THGCal/interface/veryfrontend_emulator/HGCalROCEmulImpl.h"

HGCalROCEmulImpl::HGCalROCEmulImpl(const edm::ParameterSet& conf)
{
  edm::ParameterSet sumconf = conf.getParameter<edm::ParameterSet>("summationCfg");
  double lsb_silicon_fC = sumconf.getParameter<double>("siliconCellLSB_fC") ;
  double lsb_scintillator_MIP = sumconf.getParameter<double>("scintillatorCellLSB_MIP"); //actually in fC the naming with _MIP is confusing
  
  //1 sigma noise of silicon in fC
  constexpr unsigned nThickness = 3;
  std::vector<double> thresholds_silicon = sumconf.getParameter<edm::ParameterSet>("noiseSilicon").getParameter<std::vector<double>>("values");  

  //check for array size
  if (thresholds_silicon.size() != nThickness) 
    throw cms::Exception("Configuration") << thresholds_silicon.size() << " silicon thresholds are given instead of "<< nThickness << " (the number of sensor thicknesses)";

  //1 sigma noise of scintillator in MIP
  double threshold_scintillator = sumconf.getParameter<edm::ParameterSet>("noiseScintillator").getParameter<double>("noise_MIP");                

  //level of sigma (1,2,3..)
  double threshold = sumconf.getParameter<double>("noiseThreshold");                                                                             
  //threshold = 5;

  for(unsigned ithickness=0;ithickness<nThickness;ithickness++)
    threshold_silicon_[ithickness] = uint16_t(TMath::Nint(thresholds_silicon[ithickness]*threshold/lsb_silicon_fC));
  threshold_scintillator_ = uint16_t(TMath::Nint(threshold_scintillator*threshold/lsb_scintillator_MIP));
  
  cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl lsb_silicon_fC : " << lsb_silicon_fC << endl;
  cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl lsb_scintillator_MIP : " << lsb_scintillator_MIP << endl;
  for(unsigned ithickness=0;ithickness<nThickness;ithickness++)
    cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl thresholds_silicon["<< ithickness <<"] : " << thresholds_silicon[ithickness] << endl;
  cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl threshold_scintillator : " << threshold_scintillator << endl;
  cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl threshold : " << threshold << endl;
  for(unsigned ithickness=0;ithickness<nThickness;ithickness++)
    cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl thresholds_silicon_["<< ithickness <<"] : " << threshold_silicon_[ithickness] << endl;
  cout<<"HGCalROCEmulImpl::HGCalROCEmulImpl threshold_scintillator_ : " << threshold_scintillator_ << endl;

  configCh.pedestal_adc(0);
  configCh.threshold_adc(0);
  configCh.pedestal_tot(0);
  configCh.plateau(0);
  configCh.multfactor(25);
  configCh.tdc_onset_si(614); //equivalent to 60fC
  configCh.tdc_onset_sci(563); //equivalent to 55fC

}

void HGCalROCEmulImpl::process(const std::vector<HGCalDataFrame>& dataframes, l1t::HGCalTriggerCellBxCollection& triggerCellColl)
{
  
  constexpr int kCentral = 2;
  constexpr int kHighDensityThickness = 0;
  std::unordered_map<uint32_t, uint32_t> triggercells;
  for (const auto& frame : dataframes) {
    
    int thickness = triggerTools_.thicknessIndex(frame.id());
    bool isldm = (thickness!=kHighDensityThickness) ? true : false;
    bool isScintillator = triggerTools_.isScintillator(frame.id());
    uint16_t thres = (isScintillator) ? threshold_scintillator_ : threshold_silicon_[thickness];
    
    configCh.data(frame[kCentral].data());
    configCh.mode(frame[kCentral].mode());
    configCh.ldm(isldm);    
    configCh.sci(isScintillator);
    configCh.threshold_adc(thres);

    hgcal_roc::DataChannelROC dataroc(configCh);
    
    uint32_t tcid = triggerTools_.getTriggerGeometry()->getTriggerCellFromCell(frame.id());
    triggercells.emplace(tcid , 0); // do nothing if key exists already
    // int zside = triggerTools_.zside(frame.id());
    // // sums energy for the same trigger cell id
    // if(zside>0.0)
    //   cout<<"HGCalROCEmulImpl::process detidid : "<< uint32_t(frame.id()) <<", tcid : "<< tcid <<", mode : "<< frame[kCentral].mode() << ", digival : "<<frame[kCentral].data()
    // 	  <<", linearized : "<< dataroc.linearize() <<", thres : "<< thres <<endl;
    triggercells[tcid] += dataroc.linearize();
    if(triggercells[tcid] > configCh.tcdata_max()) triggercells[tcid] = configCh.tcdata_max();
  }
  
  for (const auto& [tcid, uncompval]  : triggercells) {    

    if(uncompval==0) continue;
    
    GlobalPoint tcpos = triggerTools_.getTCPosition(tcid);
    int thickness = triggerTools_.thicknessIndex(DetId(tcid));
    bool isldm = (thickness!=kHighDensityThickness) ? true : false;
    configCh.ldm(isldm);    
    hgcal_roc::DataChannelTC tccomp(configCh);
    int zside = triggerTools_.zside(tcid);
    if(zside>0.0)
      cout<<"HGCalROCEmulImpl::process tcid : "<<tcid<<", thickness : "<< thickness <<", uncompressed value : "<<uncompval<<", compressed : "<< tccomp.compress(uncompval) <<", re-uncompressed : "<< tccomp.decoding(tccomp.compress(uncompval)) <<endl;
    l1t::HGCalTriggerCell tc(reco::LeafCandidate::LorentzVector(), 0, 0, 0, 0, 0);
    math::PtEtaPhiMLorentzVector p4((double)uncompval / cosh(tcpos.eta()), tcpos.eta(), tcpos.phi(), 0.); //The p4 setup is dummy one
    tc.setP4(p4);
    tc.setDetId(tcid);
    tc.setPosition(tcpos);
    tc.setUncompressedCharge(uncompval);
    tc.setCompressedCharge(tccomp.compress(uncompval));
    triggerCellColl.push_back(0, tc);

  }
}
