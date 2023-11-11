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

  MonitorElement *h_trigtime, *h_trigtype, *h_ECONTRawDataErrors;

  // ------------ member function ------------
  bool is0xFECAFE(const uint32_t word);
  int find0xFECAFE(const uint64_t* payload, const size_t payload_size, int n);
  void eventDump(const uint64_t* payload, const size_t payload_size, edm::LogSystem& out, int verbose);
  void getRawLocations(uint32_t packet_locations[12], uint32_t* packet);
  uint32_t pick_bits32(uint32_t number, int start_bit, int number_of_bits);
  void getRawEnergies(uint64_t packet_energies[12], uint32_t* packet);
  uint64_t pick_bits64(uint64_t number, int start_bit, int number_of_bits);

  // ------------ member data ------------
  const edm::EDGetTokenT<FEDRawDataCollection> trigRawToken_;
  const edm::EDGetTokenT<HGCalTestSystemMetaData> metadataToken_;
  edm::ESGetToken<HGCalCondSerializableModuleInfo, HGCalCondSerializableModuleInfoRcd> moduleInfoToken_;
  edm::ESGetToken<HGCalCondSerializableSiCellChannelInfo, HGCalCondSerializableSiCellChannelInfoRcd> siModuleInfoToken_;
  std::map<MonitorKey_t, MonitorKey_t> module_keys_;
  std::map<uint32_t, HGCalSiCellChannelInfo> eleidtoSiInfo_;

  const bool debug_;
  const int
      verbose_;  //level 5 (all), 4(frequent erros), 3(errors), 2(rare errors), 1(very rare errors/error under study)
  unsigned runNumber_;
  const unsigned prescale_;  // set to 1 to fill histogram for every event
  const unsigned min_num_evts_;
  unsigned num_processed_ = 0;
};

HGCalTriggerClient::HGCalTriggerClient(const edm::ParameterSet& iConfig)
    : trigRawToken_(consumes<FEDRawDataCollection>(iConfig.getParameter<edm::InputTag>("RawTrigData"))),
      metadataToken_(consumes<HGCalTestSystemMetaData>(iConfig.getParameter<edm::InputTag>("MetaData"))),
      //recRun_(consumes<hgcal::SlinkFromRaw>(iConfig.getParameter<edm::InputTag>("RecRun"))),
      moduleInfoToken_(
          esConsumes<HGCalCondSerializableModuleInfo, HGCalCondSerializableModuleInfoRcd, edm::Transition::BeginRun>()),
      siModuleInfoToken_(esConsumes<HGCalCondSerializableSiCellChannelInfo,
                                    HGCalCondSerializableSiCellChannelInfoRcd,
                                    edm::Transition::BeginRun>()),
      debug_(iConfig.getUntrackedParameter<bool>("debug")),
      verbose_(iConfig.getUntrackedParameter<int>("verbose")),
      runNumber_(iConfig.getUntrackedParameter<unsigned>("runNumber")),
      prescale_(std::max(1u, iConfig.getParameter<unsigned>("Prescale"))),
      min_num_evts_(iConfig.getParameter<unsigned>("MinimumNumEvents")) {}

HGCalTriggerClient::~HGCalTriggerClient() {
  LogDebug("HGCalTriggerClient") << "End of the job"
                                 << "\n";
}

// print 64-bit words of event
void HGCalTriggerClient::eventDump(const uint64_t* payload,
                                   const size_t payload_size,
                                   edm::LogSystem& out,
                                   int verbose = 5) {
  if (verbose_ >= verbose) {
    for (unsigned i = 0; i < payload_size; i++) {
      out << std::setw(4) << i << "  ";
      out << std::hex << std::setfill('0');
      out << std::setw(16) << payload[i] << '\n';
      out << std::dec << std::setfill(' ');
    }
  }
  out << "\n";
}

//Checks of matches with 0xFECAFE
bool HGCalTriggerClient::is0xFECAFE(const uint32_t word) {
  if (((word >> 8) & 0xFFFFFF) == 16698110)
    return true;  //0xFECAFE
  else
    return false;
}

// returns location of the n'th 0xFECAFE
int HGCalTriggerClient::find0xFECAFE(const uint64_t* payload, const size_t payload_size, int n) {
  int cafe_counter = 0;
  int cafe_word_idx = -1;
  for (unsigned i(0); i < payload_size; i++) {
    const uint32_t word = payload[i];
    if (is0xFECAFE(word)) {  // if word == 0xfeca
      cafe_counter++;
      if (cafe_counter == n) {
        cafe_word_idx = i;
        break;
      }
    }
  }

  if (cafe_word_idx == -1) {
    //Could not find 0xfecafe word;
    return 0;
  } else {
    return cafe_word_idx;
  }
}

// Pick bits
uint32_t HGCalTriggerClient::pick_bits32(uint32_t number, int start_bit, int number_of_bits) {
  // Create a mask to extract the desired bits.
  uint32_t mask = (1 << number_of_bits) - 1;
  // Shift the number to the start bit position.
  number = number >> (32 - start_bit - number_of_bits);
  // Mask the number to extract the desired bits.
  uint32_t picked_bits = number & mask;

  return picked_bits;
}

// 12 locations, 2 bits long, immediately after the packet counter
void HGCalTriggerClient::getRawLocations(uint32_t packet_locations[12], uint32_t* packet) {
  for (int i = 0; i < 12; i++) {
    packet_locations[i] = pick_bits32(packet[0], 4 + i * 2, 2);
  }
}

// Pick bits
uint64_t HGCalTriggerClient::pick_bits64(uint64_t number, int start_bit, int number_of_bits) {
  // Create a mask to extract the desired bits.
  uint64_t mask = (1 << number_of_bits) - 1;
  // Shift the number to the start bit position.
  number = number >> (64 - start_bit - number_of_bits);
  // Mask the number to extract the desired bits.
  uint64_t picked_bits = number & mask;

  return picked_bits;
}

// 12 energies, 7 bits long, immediately after the packet energies
void HGCalTriggerClient::getRawEnergies(uint64_t packet_energies[12], uint32_t* packet) {
  uint64_t packet64[4];
  for (int i = 0; i < 4; i++) {
    packet64[i] = packet[i];
  }

  // need two 64 bit words since all of the energies are 12*7 = 84 bits long
  // word 1 starts with the beginning of the energies
  uint64_t word1 = (packet64[0] << (28 + 32)) + (packet64[1] << 28) + (packet64[2] >> 4);
  // word 2 are the last 64 bits of the packet (which is 128 bits long)
  uint64_t word2 = (packet64[2] << 32) + packet64[3];

  for (int i = 0; i < 12; i++) {
    if (i < 9) {
      // first 9 (0->8) energies fit in first word
      packet_energies[i] = pick_bits64(word1, i * 7, 7);
    } else {
      // 9th energy starts 27 bits into the second word
      //packet_energies[i] = pick_bits64(word2, 27+(9-i)*7, 7); //problem
      packet_energies[i] = pick_bits64(word2, 27 + (i - 9) * 7, 7);  //modification by Indra
    }
  }
}

void HGCalTriggerClient::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  bool toProcess = (num_processed_ < min_num_evts_) || (num_processed_ % prescale_ == 0);
  ++num_processed_;
  if (!toProcess)
    return;

  // fill histogram for trigtime distribution
  const auto& metadata = iEvent.get(metadataToken_);
  uint32_t trigTime = metadata.trigTime_;
  h_trigtime->Fill(int(trigTime));

  uint32_t trigType = metadata.trigType_;

  // read ECON-T raw data
  const auto& raw_trig_data = iEvent.get(trigRawToken_);
  const auto& data = raw_trig_data.FEDData(0);

  // const auto recEvent = iEvent.get(recRun_);
  // const hgcal_slinkfromraw::RecordRunning *recRecord =  recEvent->getTrigRecRun();

  // vector<char> ==> uint64_t*
  const uint64_t* payload = (uint64_t*)(data.data());
  const size_t payload_size = data.size() / sizeof(uint64_t);

  edm::LogSystem out("HGCalTriggerClient");
  if (debug_)
    eventDump(payload, payload_size, out, 5);

  // ====== ECON-T unpacker ======

  bool isBxE0 = false;
  bool isBxE1 = false;
  // bool isBxCE0 = false;
  // bool isBxCE1 = false;
  bool isSTCNE0 = false;
  bool isSTCNE1 = false;
  bool isSTCLE0 = false;
  bool isSTCLE1 = false;
  bool isEngE0 = false;
  bool isEngE1 = false;

  if (trigType == 0x0001)
    h_trigtype->Fill(0);
  if (trigType == 0x0002)
    h_trigtype->Fill(1);
  if (trigType == 0x0004)
    h_trigtype->Fill(2);
  if (trigType == 0x0008)
    h_trigtype->Fill(3);
  if (trigType == 0x0010)
    h_trigtype->Fill(4);

  uint16_t daq_data[5];        //5 : data blocks separated by 0xfecafe
  uint16_t daq_nbx[5];         //5 : data blocks separated by 0xfecafe
  uint16_t size_in_cafe[5];    //5 : data blocks separated by 0xfecafe
  uint16_t daq_event_size[5];  //5 : data blocks separated by 0xfecafe
  int cafe_word_loc[5];        //5 : 0xfecafe's

  for (unsigned i(0); i < 5; i++) {
    daq_data[i] = payload[2] >> (i * 7) & 0xF;
    daq_nbx[i] = payload[2] >> (i * 7 + 4) & 0x7;
    daq_event_size[i] = (2 * daq_nbx[i] + 1) * daq_data[i];
    cafe_word_loc[i] = find0xFECAFE(payload, payload_size, i + 1);
    size_in_cafe[i] = payload[cafe_word_loc[i]] & 0xFF;
  }

  if (verbose_ >= 5) {
    //std::cout << "EventId : " << (recRecord->slinkBoe())->eventId() << std::endl;
    out << "Processing Event number : " << iEvent.id().event() << "\n";
    out << "daq0_event_size : " << daq_event_size[0] << ", size_in_cafe[0] : " << size_in_cafe[0] << "\n";
    out << "first_cafe_word_loc : " << cafe_word_loc[0] << "\n";
  }

  int sixth_cafe_word_loc = find0xFECAFE(payload, payload_size, 6);
  if (sixth_cafe_word_loc != 0) {
    //nofExcessFECAFEErrors++ ;
    h_ECONTRawDataErrors->Fill(0);
    return;
  }

  if (cafe_word_loc[0] != 3) {
    if (verbose_ >= 1)
      out << "Event : " << iEvent.id().event()
          << ", Corrupted header need to skip event as the first 0xfecafe word is at  " << cafe_word_loc[0]
          << " instead of ideal location 3."
          << "\n";
    //nofFirstFECAFEErrors++ ;
    h_ECONTRawDataErrors->Fill(1);
    return;
  }

  for (unsigned i(0); i < 5; i++) {
    if (daq_event_size[i] != size_in_cafe[i]) {
      if (verbose_ >= 2)
        out << "Event : " << iEvent.id().event() << ", Event size do not match between trigger RO header "
            << daq_event_size[i] << " and " << i << "-th 0xfecafe word " << size_in_cafe[i] << "\n";
      h_ECONTRawDataErrors->Fill(2);
      return;
    }
  }

  if (daq_nbx[0] != daq_nbx[1]) {
    if (verbose_ >= 1)
      out << "Event : " << iEvent.id().event() << ", Bx size do not match between packed " << daq_nbx[0]
          << " and unpacked data " << daq_nbx[1] << std::endl;
    //nofNbxMisMatches++;
    h_ECONTRawDataErrors->Fill(3);
    return;
  }

  //////////////////// NOTE : The following raw decoding is only valid for STC mode ///////////////////////

  int bx_index = -1.0 * int(daq_nbx[0]);
  const int maxnbx = (2 * daq_nbx[0] + 1);  //In case of raw input
  uint32_t energy_raw[2][maxnbx][12];       //2 : LSB/MSB, 12 : STC
  uint32_t loc_raw[2][maxnbx][12];          //2 : LSB/MSB, 12 : STC
  uint32_t bx_raw[2][maxnbx][12];           //2 : LSB/MSB, 15 : STC
  uint32_t packet[4];                       //a bx packet constitutes 4 64 bit words
  uint32_t packet_locations[12];
  uint64_t packet_energies[12];

  for (int iect = 0; iect < 2; iect++) {
    for (int ibx = 0; ibx < maxnbx; ibx++) {
      for (int istc = 0; istc < 12; istc++) {
        energy_raw[iect][ibx][istc] = 0;
        loc_raw[iect][ibx][istc] = 0;
        bx_raw[iect][ibx][istc] = 0;
      }
    }
  }

  //Decode unpacker input rawdata
  for (unsigned i(cafe_word_loc[0] + 1); i < daq_event_size[0] + unsigned(cafe_word_loc[0]) + 1; i = i + 4) {
    //Get LSB raw values
    const uint32_t wordL = (payload[i] & 0xFFFFFFFF);
    const uint32_t bx_counterL = (wordL >> 28) & 0xF;

    packet[0] = (payload[i] & 0xFFFFFFFF);
    packet[1] = (payload[i + 1] & 0xFFFFFFFF);
    packet[2] = (payload[i + 2] & 0xFFFFFFFF);
    packet[3] = (payload[i + 3] & 0xFFFFFFFF);

    getRawLocations(packet_locations, packet);
    getRawEnergies(packet_energies, packet);

    for (int istc = 0; istc < 12; istc++) {
      int ibx = bx_index + int(daq_nbx[0]);
      energy_raw[0][ibx][istc] = packet_energies[istc];
      loc_raw[0][ibx][istc] = packet_locations[istc];
      bx_raw[0][ibx][istc] = bx_counterL;
      if (verbose_ >= 3) {
        out << "Event : " << iEvent.id().event() << ", STCL " << istc << ", ibxL " << ibx
            << ", EnergyL : " << energy_raw[0][ibx][istc] << ", locL : " << loc_raw[0][ibx][istc]
            << ", bx_counterL : " << bx_raw[0][ibx][istc] << "\n";
      }
    }

    //Get MSB raw values
    const uint32_t wordM = ((payload[i] >> 32) & 0xFFFFFFFF);
    const uint32_t bx_counterM = (wordM >> 28) & 0xF;

    packet[0] = ((payload[i] >> 32) & 0xFFFFFFFF);
    packet[1] = ((payload[i + 1] >> 32) & 0xFFFFFFFF);
    packet[2] = ((payload[i + 2] >> 32) & 0xFFFFFFFF);
    packet[3] = ((payload[i + 3] >> 32) & 0xFFFFFFFF);

    getRawLocations(packet_locations, packet);
    getRawEnergies(packet_energies, packet);

    for (int istc = 0; istc < 12; istc++) {
      int ibx = bx_index + int(daq_nbx[0]);
      energy_raw[1][ibx][istc] = packet_energies[istc];
      loc_raw[1][ibx][istc] = packet_locations[istc];
      bx_raw[1][ibx][istc] = bx_counterM;
      if (verbose_ >= 3) {
        out << "Event : " << iEvent.id().event() << ", STCM " << istc << ", ibxM " << ibx
            << ", EnergyM : " << energy_raw[1][ibx][istc] << ", locM : " << loc_raw[1][ibx][istc]
            << ", bx_counterM : " << bx_raw[1][ibx][istc] << "\n";
      }
    }

    bx_index++;
  }

  uint32_t energy_unpkd[2][maxnbx][12];
  uint32_t loc_unpkd[2][maxnbx][12];  //2 : LSB/MSB, 12 : STC
  uint32_t bx_unpkd[2][maxnbx][12];   //2 : LSB/MSB, 15 : STC
  for (int iect = 0; iect < 2; iect++)
    for (int ibx = 0; ibx < maxnbx; ibx++)
      for (int istc = 0; istc < 12; istc++) {
        energy_unpkd[iect][ibx][istc] = 0;
        loc_unpkd[iect][ibx][istc] = 0;
        bx_unpkd[iect][ibx][istc] = 0;
      }

  int index_stc = 0;
  int index_ibx = 0;

  //Decode unpacker output
  for (unsigned i(cafe_word_loc[1] + 1); i < daq_event_size[1] + unsigned(cafe_word_loc[1]) + 1; i++) {
    const uint32_t wordL = (payload[i] & 0xFFFFFFFF);
    const uint32_t wordM = ((payload[i] >> 32) & 0xFFFFFFFF);

    const uint32_t energy1 = wordL & 0x7F;
    const uint32_t location1 = (wordL >> 7) & 0x3F;
    const uint32_t energy2 = (wordL >> 13) & 0x7F;
    const uint32_t location2 = (wordL >> (13 + 7)) & 0x3F;
    const uint32_t bx_counter = (wordL >> 26) & 0xF;

    const uint32_t energy3 = wordM & 0x7F;
    const uint32_t location3 = (wordM >> 7) & 0x3F;
    const uint32_t energy4 = (wordM >> 13) & 0x7F;
    const uint32_t location4 = (wordM >> (13 + 7)) & 0x3F;
    const uint32_t bx_counter1 = (wordM >> 26) & 0xF;

    energy_unpkd[0][index_ibx][index_stc] = energy1;
    energy_unpkd[0][index_ibx][index_stc + 6] = energy2;
    energy_unpkd[1][index_ibx][index_stc] = energy3;
    energy_unpkd[1][index_ibx][index_stc + 6] = energy4;

    loc_unpkd[0][index_ibx][index_stc] = location1;
    loc_unpkd[0][index_ibx][index_stc + 6] = location2;
    loc_unpkd[1][index_ibx][index_stc] = location3;
    loc_unpkd[1][index_ibx][index_stc + 6] = location4;

    bx_unpkd[0][index_ibx][index_stc] = bx_counter;
    bx_unpkd[0][index_ibx][index_stc + 6] = bx_counter;
    bx_unpkd[1][index_ibx][index_stc] = bx_counter1;
    bx_unpkd[1][index_ibx][index_stc + 6] = bx_counter1;

    if (verbose_ >= 3) {
      out << "Event : " << iEvent.id().event() << "LSB :  STC1 " << index_stc << ", STC2 " << index_stc + 6 << ", ibx "
          << index_ibx << ", EnergyL1 : " << energy_unpkd[0][index_ibx][index_stc]
          << ", locL1 : " << (loc_unpkd[0][index_ibx][index_stc] & 0x3)
          << ", bx_counterL1 : " << bx_unpkd[0][index_ibx][index_stc] << "\n"
          << "\t\tEnergyL2 : " << energy_unpkd[0][index_ibx][index_stc + 6]
          << ", locL2 : " << (loc_unpkd[0][index_ibx][index_stc + 6] & 0x3)
          << ", bx_counterL2 : " << bx_unpkd[0][index_ibx][index_stc + 6] << "\n";
      out << "\t MSB :  STC3 " << index_stc << ", STC4 " << index_stc + 6 << ", ibx " << index_ibx
          << ", EnergyL3 : " << energy_unpkd[1][index_ibx][index_stc]
          << ", locL3 : " << (loc_unpkd[1][index_ibx][index_stc] & 0x3)
          << ", bx_counterL3 : " << bx_unpkd[1][index_ibx][index_stc] << "\n"
          << "\t\tEnergyL4 : " << energy_unpkd[1][index_ibx][index_stc + 6]
          << ", locL4 : " << (loc_unpkd[1][index_ibx][index_stc + 6] & 0x3)
          << ", bx_counterL4 : " << bx_unpkd[1][index_ibx][index_stc + 6] << "\n";
    }

    if (index_stc == 5)
      index_stc = 0;
    else
      index_stc++;
    if (index_stc == 0)
      index_ibx++;
  }

  //check bx
  for (int iect = 0; iect < 2; iect++) {
    for (int ibx = 0; ibx < maxnbx; ibx++) {
      for (int istc = 0; istc < 12; istc++) {
        if (bx_unpkd[iect][ibx][istc] != bx_raw[iect][ibx][istc]) {
          if (verbose_ >= 4)
            std::cerr << " Unpacked location value do not match with the packed one for (Run, event, iecont, bx, stc, "
                         "bx_unpacked, bx_packed ) : "
                      << "(" << runNumber_ << "," << iEvent.id().event() << "," << iect << "," << ibx << "," << istc
                      << "," << (bx_unpkd[iect][ibx][istc] & 0x3) << "," << bx_raw[iect][ibx][istc] << ") "
                      << std::endl;
          if (iect == 0) {
            //nofBxRawUnpkMM0++;
            isBxE0 = true;
          } else {
            //nofBxRawUnpkMM1++;
            isBxE1 = true;
          }
        }
      }
    }
    // The following test can not be performed untill we can access HGCalSlinkFromRaw
    // //modulo test
    // if(ev.bxId==3564){
    //   if(bx_raw[iect][daq_nbx[0]][0]!=15 || bx_unpkd[iect][daq_nbx[1]][0]!=15){
    // 	//std::cerr << "Bx Module test failed for iect : "<< iect <<" since ev.bxId%8 : " << ev.bxId <<" and bx_raw : "<< bx_raw[iect][daq_nbx[0]][0] << " and bx_unpkd : "<< bx_unpkd[iect][daq_nbx[1]][0] << std::endl ;
    // 	isGood = false;
    // 	if(iect==0){
    // 	  nofBxCentralMM++;
    // 	  isBxCE = true;
    // 	}else{
    // 	  nofBxCentralMM1++;
    // 	  isBxCE1 = true;
    // 	}
    //   }
    // }else{
    //   if((ev.bxId%8 != bx_raw[iect][daq_nbx[0]][0]) || (ev.bxId%8 != bx_unpkd[iect][daq_nbx[1]][0])){
    // 	//std::cerr << "Bx Module test failed for iect : "<< iect <<" since ev.bxId%8 : " << (ev.bxId%8) <<" and bx_raw : "<< bx_raw[iect][daq_nbx[0]][0] << " and bx_unpkd : "<< bx_unpkd[iect][daq_nbx[1]][0] << std::endl ;
    // 	isGood = false;
    // 	if(iect==0) {
    // 	  nofBxCentralMM++;
    // 	  isBxCE = true;
    // 	}else{
    // 	  nofBxCentralMM1++;
    // 	  isBxCE1 = true;
    // 	}
    //   }
    // }
  }

  //Check Locations
  for (int iect = 0; iect < 2; iect++) {
    for (int ibx = 0; ibx < maxnbx; ibx++) {
      for (int istc = 0; istc < 12; istc++) {
        if ((loc_unpkd[iect][ibx][istc] >> 2 & 0xF) != unsigned(istc)) {
          if (verbose_ >= 4)
            std::cerr << " Unpacked location index do not match with the STC for (Run, event, iecont, bx, stc, "
                         "istc_from_unpacked) : "
                      << "(" << runNumber_ << "," << iEvent.id().event() << "," << iect << "," << ibx << "," << istc
                      << "," << (loc_unpkd[iect][ibx][istc] >> 2 & 0xF) << ") " << std::endl;
          if (iect == 0) {
            isSTCNE0 = true;
          } else {
            isSTCNE1 = true;
          }
        }
        if ((loc_unpkd[iect][ibx][istc] & 0x3) != loc_raw[iect][ibx][istc]) {
          if (verbose_ >= 4)
            std::cerr << " Unpacked location value do not match with the packed one for (Run, event, iecont, bx, stc, "
                         "loc_unpacked, loc_packed ) : "
                      << "(" << runNumber_ << "," << iEvent.id().event() << "," << iect << "," << ibx << "," << istc
                      << "," << (loc_unpkd[iect][ibx][istc] & 0x3) << "," << loc_raw[iect][ibx][istc] << ") "
                      << std::endl;
          if (iect == 0) {
            isSTCLE0 = true;
          } else {
            isSTCLE1 = true;
          }
        }
      }
    }
  }

  //Check Energy
  for (int iect = 0; iect < 2; iect++) {
    for (int ibx = 0; ibx < maxnbx; ibx++) {
      for (int istc = 0; istc < 12; istc++) {
        if (energy_raw[iect][ibx][istc] != energy_unpkd[iect][ibx][istc]) {
          if (verbose_ >= 4)
            std::cerr << " Packed and unpacked energies does not match for (Run, "
                         "event,iecont,bx.stc,raw_energy,unpacked_energy) : "
                      << "(" << runNumber_ << "," << iEvent.id().event() << "," << iect << "," << ibx << "," << istc
                      << "," << energy_raw[iect][ibx][istc] << "," << energy_unpkd[iect][ibx][istc] << ") "
                      << std::endl;

          if (iect == 0) {
            isEngE0 = true;
          } else {
            isEngE1 = true;
          }
        }
      }
    }
  }

  if (isSTCNE0)
    h_ECONTRawDataErrors->Fill(4);
  if (isSTCLE0)
    h_ECONTRawDataErrors->Fill(5);
  if (isEngE0)
    h_ECONTRawDataErrors->Fill(6);
  if (isBxE0)
    h_ECONTRawDataErrors->Fill(7);
  //if(isBxCE0) nofEvcBxCE0++;

  if (isSTCNE1)
    h_ECONTRawDataErrors->Fill(8);
  if (isSTCLE1)
    h_ECONTRawDataErrors->Fill(9);
  if (isEngE1)
    h_ECONTRawDataErrors->Fill(10);
  if (isBxE1)
    h_ECONTRawDataErrors->Fill(11);
  //if(isBxCE1) nofEvcBxCE1++;

  //////////////////// NOTE : The above raw decoding is only valid for STC mode ///////////////////////
}

void HGCalTriggerClient::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
  //book monitoring elements (histos, profiles, etc.)
  ibook.setCurrentFolder("HGCAL/Trigger");
  h_trigtime = ibook.book1D("trigtime", ";trigger phase; Counts", 200, 0, 200);

  //ibook.setCurrentFolder("HGCAL/Trigger");
  h_trigtype = ibook.book1D("trigtype", ";L1a trigger types", 5, 0, 5);
  h_trigtype->setBinLabel(1, "Phys");
  h_trigtype->setBinLabel(2, "Calib");
  h_trigtype->setBinLabel(3, "random");
  h_trigtype->setBinLabel(4, "soft");
  h_trigtype->setBinLabel(5, "regular");

  h_ECONTRawDataErrors = ibook.book1D("ECONTRawDataErrors", ";", 12, 0, 12);
  h_ECONTRawDataErrors->setBinLabel(1, "NofFECAFE>5");  //5 0xfecafe have been used in September'23 beamtest
  h_ECONTRawDataErrors->setBinLabel(2, "1stFECAFE");    //1st 0xfecafe in wrong position
  h_ECONTRawDataErrors->setBinLabel(
      3, "EvtSizeMM");  //Event size mismatch between that mentioned in header vs in the 0xfecafe 8 bit LSB position
  h_ECONTRawDataErrors->setBinLabel(4, "NbxMM");  //Number of bx's are not maching between unpacker input and output
  h_ECONTRawDataErrors->setBinLabel(
      5,
      "STC-index(LSB)");  //The STC index as written in the unpacked data does not match with the location where STC information is written in the rawdata packet (for LSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      6,
      "STC location(LSB)");  //The location of STC as written in unpacker input does not match with the unpacked data (for LSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      7,
      "STC Energy(LSB)");  //The energy of STC as written in unpacker input does not match with the unpacked data (for LSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      8,
      "Bx-index(LSB)");  //The bx index as written in unpacker input does not match with the unpacked data (for LSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      9,
      "STC-index(MSB)");  //The STC index as written in the unpacked data does not match with the location where STC information is written in the rawdata packet (for MSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      10,
      "STC location(MSB)");  //The location of STC as written in unpacker input does not match with the unpacked data (for MSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      11,
      "STC Energy(MSB)");  //The energy of STC as written in unpacker input does not match with the unpacked data (for MSB 32-bit word)
  h_ECONTRawDataErrors->setBinLabel(
      12,
      "Bx-index(MSB)");  //The bx index as written in unpacker input does not match with the unpacked data (for MSB 32-bit word)
}

void HGCalTriggerClient::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("RawTrigData", edm::InputTag("hgcalEmulatedSlinkRawData", "hgcalTriggerRawData"));
  desc.add<edm::InputTag>("MetaData", edm::InputTag("hgcalEmulatedSlinkRawData", "hgcalMetaData"));
  //desc.add<edm::InputTag>("RecRun", edm::InputTag("hgcalEmulatedSlinkRawData", "hgcalRecRun"));
  desc.addUntracked<bool>("debug", true);
  desc.addUntracked<unsigned>("runNumber", 0);
  desc.addUntracked<int>("verbose", 1);
  desc.add<unsigned>("Prescale", 1);
  desc.add<unsigned>("MinimumNumEvents", 10000);
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(HGCalTriggerClient);
