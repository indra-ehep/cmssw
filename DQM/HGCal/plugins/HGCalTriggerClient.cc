#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"

#include "DataFormats/HGCalDigi/interface/HGCalElectronicsId.h"
#include "DataFormats/HGCalDigi/interface/HGCalTestSystemMetadata.h"

#include "CondFormats/DataRecord/interface/HGCalCondSerializableModuleInfoRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalCondSerializableModuleInfo.h"
#include "CondFormats/DataRecord/interface/HGCalCondSerializableSiCellChannelInfoRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalCondSerializableSiCellChannelInfo.h"
#include "Geometry/HGCalMapping/interface/HGCalElectronicsMappingTools.h"

#include <string>
#include <fstream>
#include <iostream>
#include <map>

class HGCalTriggerClient : public DQMEDAnalyzer {
public:
  typedef HGCalCondSerializableModuleInfo::ModuleInfoKey_t MonitorKey_t;

  explicit HGCalTriggerClient(const edm::ParameterSet&);
  ~HGCalTriggerClient() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  /**
     @short takes care of booking the monitoring elements at the start of the run
     the histograms are instantiated per module according to the module mapping
     received from the event setup
   */
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;

  /**
     @short histogram filler per event
   */
  void analyze(const edm::Event&, const edm::EventSetup&) override;

  MonitorElement* h_trigtime;
  // MonitorElement *p_econdquality, *p_econdcbquality, *p_econdpayload;

  // ------------ member data ------------
  const edm::EDGetTokenT<FEDRawDataCollection> trigRawToken_;
  const edm::EDGetTokenT<HGCalTestSystemMetaData> metadataToken_;
  edm::ESGetToken<HGCalCondSerializableModuleInfo, HGCalCondSerializableModuleInfoRcd> moduleInfoToken_;
  edm::ESGetToken<HGCalCondSerializableSiCellChannelInfo, HGCalCondSerializableSiCellChannelInfoRcd> siModuleInfoToken_;
  std::map<MonitorKey_t, MonitorKey_t> module_keys_;
  std::map<uint32_t, HGCalSiCellChannelInfo> eleidtoSiInfo_;

  const bool debug_;
  const unsigned prescale_;
  const unsigned min_num_evts_;
  unsigned num_processed_ = 0;
};

HGCalTriggerClient::HGCalTriggerClient(const edm::ParameterSet& iConfig)
    : trigRawToken_(consumes<FEDRawDataCollection>(iConfig.getParameter<edm::InputTag>("RawTrigData"))),
      metadataToken_(consumes<HGCalTestSystemMetaData>(iConfig.getParameter<edm::InputTag>("MetaData"))),
      moduleInfoToken_(
          esConsumes<HGCalCondSerializableModuleInfo, HGCalCondSerializableModuleInfoRcd, edm::Transition::BeginRun>()),
      siModuleInfoToken_(esConsumes<HGCalCondSerializableSiCellChannelInfo,
                                    HGCalCondSerializableSiCellChannelInfoRcd,
                                    edm::Transition::BeginRun>()),
      debug_(iConfig.getUntrackedParameter<bool>("debug")),
      prescale_(std::max(1u, iConfig.getParameter<unsigned>("Prescale"))),
      min_num_evts_(iConfig.getParameter<unsigned>("MinimumNumEvents")) {}

HGCalTriggerClient::~HGCalTriggerClient() { LogDebug("HGCalTriggerClient") << "End of the job" << std::endl; }

void HGCalTriggerClient::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  bool toProcess = (num_processed_ < min_num_evts_) || (num_processed_ % prescale_ == 0);
  ++num_processed_;
  if (!toProcess)
    return;

  // fill histogram for trigtime distribution
  const auto& metadata = iEvent.get(metadataToken_);
  int trigTime = metadata.trigTime_;
  h_trigtime->Fill(trigTime);

  // read ECON-T raw data
  const auto& raw_trig_data = iEvent.get(trigRawToken_);
  const auto& data = raw_trig_data.FEDData(0);

  // vector<char> ==> uint64_t*
  const uint64_t* payload = (uint64_t*)(data.data());
  const size_t payload_size = data.size() / sizeof(uint64_t);

  edm::LogSystem out("HGCalTriggerClient");
  if (debug_) {
    out << std::hex << std::setfill('0');
    for (unsigned i = 0; i < payload_size; i++) {
      out << std::setw(4) << i << "  " << std::setw(16) << payload[i] << '\n';
    }
    out << std::dec << std::setfill(' ');
  }

  // ====== TODO: ECON-T unpacker ======
  // ...
  // ====== TODO: ECON-T unpacker ======

  // //read flagged ECON-D list
  // const auto& flagged_econds = iEvent.getHandle(econdQualityToken_);
  // if (flagged_econds.isValid()) {
  //   for (auto econd : *flagged_econds) {
  //     HGCalElectronicsId eleid(econd.eleid);
  //     MonitorKey_t logiKey(eleid.zSide(), eleid.fedId(), eleid.captureBlock(), eleid.econdIdx());
  //     MonitorKey_t k = module_keys_[logiKey];
  //     int ibin = modbins_[k];
  //     p_econdpayload->Fill(ibin, econd.payload);
  //     p_econdcbquality->Fill(ibin, econd.cbflags);
  //     if (econd.cbFlag())
  //       p_econdquality->Fill(ibin, 0);
  //     if (econd.htFlag())
  //       p_econdquality->Fill(ibin, 1);
  //     if (econd.eboFlag())
  //       p_econdquality->Fill(ibin, 2);
  //     if (econd.matchFlag())
  //       p_econdquality->Fill(ibin, 3);
  //     if (econd.truncatedFlag())
  //       p_econdquality->Fill(ibin, 4);
  //     if (econd.wrongHeaderMarker())
  //       p_econdquality->Fill(ibin, 5);
  //     if (econd.payloadOverflows())
  //       p_econdquality->Fill(ibin, 6);
  //     if (econd.payloadMismatches())
  //       p_econdquality->Fill(ibin, 7);
  //   }
  // }
}

void HGCalTriggerClient::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
  // //create module keys
  // auto moduleInfo = iSetup.getData(moduleInfoToken_);
  // module_keys_ = moduleInfo.getAsSimplifiedModuleLocatorMap(true);
  // size_t nmods = module_keys_.size();
  // LogDebug("HGCalTriggerClient") << "Read module info with " << nmods << " entries";

  // //map also the cell types
  // const auto& siCellInfo = iSetup.getData(siModuleInfoToken_);
  // eleidtoSiInfo_ = hgcal::mapEleIdToSiInfo(moduleInfo, siCellInfo);

  //book monitoring elements (histos, profiles, etc.)
  ibook.setCurrentFolder("HGCAL/Trigger");
  h_trigtime = ibook.book1D("trigtime", ";trigger phase; Counts", 200, 0, 200);

  // p_econdquality = ibook.book2D("p_econdquality", ";ECON-D;Header quality flags", nmods, 0, nmods, 8, 0, 8);
  // p_econdquality->setBinLabel(1, "CB", 2);
  // p_econdquality->setBinLabel(2, "H/T", 2);
  // p_econdquality->setBinLabel(3, "E/B/O", 2);
  // p_econdquality->setBinLabel(4, "M", 2);
  // p_econdquality->setBinLabel(5, "Trunc", 2);
  // p_econdquality->setBinLabel(6, "Marker", 2);
  // p_econdquality->setBinLabel(7, "Payload (OF)", 2);
  // p_econdquality->setBinLabel(8, "Payload (mismatch)", 2);

  // p_econdcbquality = ibook.book2D("p_econdcbquality", ";ECON-D;DAQ quality flags", nmods, 0, nmods, 8, 0, 8);
  // p_econdcbquality->setBinLabel(1, "Normal ", 2);
  // p_econdcbquality->setBinLabel(2, "Payload", 2);
  // p_econdcbquality->setBinLabel(3, "CRC Error", 2);
  // p_econdcbquality->setBinLabel(4, "EvID Mis.", 2);
  // p_econdcbquality->setBinLabel(5, "FSM T/O", 2);
  // p_econdcbquality->setBinLabel(6, "BCID/OrbitID", 2);
  // p_econdcbquality->setBinLabel(7, "MB Overflow", 2);
  // p_econdcbquality->setBinLabel(8, "Innactive", 2);

  // p_econdpayload = ibook.book2D("p_econdpayload", ";ECON-D;Payload", nmods, 0, nmods, 200, 0, 500);
}

void HGCalTriggerClient::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("RawTrigData", edm::InputTag("hgcalEmulatedSlinkRawData", "hgcalTriggerRawData"));
  desc.add<edm::InputTag>("MetaData", edm::InputTag("hgcalEmulatedSlinkRawData", "hgcalMetaData"));
  desc.addUntracked<bool>("debug", true);
  desc.add<unsigned>("Prescale", 1);
  desc.add<unsigned>("MinimumNumEvents", 10000);
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(HGCalTriggerClient);
