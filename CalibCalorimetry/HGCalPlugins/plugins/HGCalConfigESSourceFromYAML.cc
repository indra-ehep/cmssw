/****************************************************************************
 *
 * This is a part of HGCal offline software.
 * Authors:
 *   Laurent Forthomme
 *
 ****************************************************************************/

#include <yaml-cpp/yaml.h>
#include <iostream> // for std::cout
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/SourceFactory.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/EventSetupRecordIntervalFinder.h"
//#include "FWCore/Framework/interface/ESProducts.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/DataRecord/interface/HGCalCondSerializableConfigRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalCondSerializableConfig.h"
//#include "CondFormats/DataRecord/interface/HGCalCondSerializableModuleInfoRcd.h"
//#include "CondFormats/HGCalObjects/interface/HGCalCondSerializableModuleInfo.h"

class HGCalConfigESSourceFromYAML : public edm::ESProducer, public edm::EventSetupRecordIntervalFinder {

public:

  explicit HGCalConfigESSourceFromYAML(const edm::ParameterSet& iConfig)
      : filename_(iConfig.getParameter<std::string>("filename")),
        charMode_(iConfig.getParameter<int>("charMode")),
        gain_(iConfig.getParameter<int>("gain")) {
    setWhatProduced(this);
    findingRecord<HGCalCondSerializableConfigRcd>();
    //LogDebug("HGCalConfigESSourceFromYAML")
    std::cout << "Init: filename=" << filename_ 
      << ", override charMode=" << charMode_ << ", override gain=" << gain_ << "..." << std::endl;
  }

  std::unique_ptr<HGCalCondSerializableConfig> produce(const HGCalCondSerializableConfigRcd&) {
    return parseYAML(filename_);
  }

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<std::string>("filename", {});
    //desc.add<edm::ESInputTag>("configSource",edm::ESInputTag(""))->setComment("Label for ROC configuration parameters");
    desc.add<int>("charMode",-1)->setComment("Manual override for characterization mode to unpack raw data");
    desc.add<int>("gain",-1)->setComment("Manual override for gain (1: 80 fC, 2: 160 fC, 4: 320 fC)");
    descriptions.addWithDefaultLabel(desc);
  }

private:

  const std::string filename_;
  const int charMode_; // manual override of YAML files
  const int gain_; // manual override of YAML files

  void setIntervalFor(const edm::eventsetup::EventSetupRecordKey&,
                      const edm::IOVSyncValue&,
                      edm::ValidityInterval& oValidity) override {
    oValidity = edm::ValidityInterval(edm::IOVSyncValue::beginOfTime(), edm::IOVSyncValue::endOfTime());
  }

  void parseNode(const std::string& node_name,
                 const YAML::Node& node,
                 std::unique_ptr<HGCalCondSerializableConfig>& cond) const {
    switch (node.Type()) {
      case YAML::NodeType::Null: {
        cond->addParameter(node_name, {});
      } break;
      case YAML::NodeType::Scalar: {
        try {
          cond->addParameter(node_name, node.as<int>());
        } catch (const YAML::BadConversion&) {
          return;
        }
      } break;
      case YAML::NodeType::Sequence:
        cond->addParameter(node_name, node.as<std::vector<int> >());
        break;
      case YAML::NodeType::Map: {
        for (const auto& subnode : node)
          parseNode(node_name + ":" + subnode.first.as<std::string>(), subnode.second, cond);
      } break;
      default:
        throw cms::Exception("HGCalConfigESSourceFromYAML")
            << "Invalid node with key='" << node_name << "': unsupported type '" << node.Type() << "'.";
    }
  }

  //void parseMetaDataNode(const std::string& node_name,
  //               const YAML::Node& node,
  //               std::unique_ptr<HGCalCondSerializableConfig>& cond) const {
  //  
  //}

  //void parseRunNode(const std::string& node_name,
  //               const YAML::Node& node,
  //               std::unique_ptr<HGCalCondSerializableConfig>& cond) const {
  //  
  //}

  std::unique_ptr<HGCalCondSerializableConfig> parseYAML(const std::string& filename) {
    auto cond = std::make_unique<HGCalCondSerializableConfig>();
    try {
      cond->moduleConfigs[0] = HGCalModuleConfig();
      
      // PARSE MAPPER
      LogDebug("HGCalConfigESSourceFromYAML") << "Loading " << filename << "...";
      const auto yaml_file = YAML::LoadFile(filename);
      const auto mapper = yaml_file["ECONs"];
      if (mapper.IsDefined()) {
        for (const auto& params : mapper) { // loop through sequence of unnamed nodes
          uint32_t id;
          try {
            id = params["id"].as<uint32_t>();
          } catch (const YAML::ParserException& err) {
            throw cms::Exception("HGCalConfigESSourceFromYAML") << "Bad conversion for id!" << err.msg;
          }
          std::string fname_ECOND = params["configs"]["ECOND"].as<std::string>();
          std::string fname_ECONT = params["configs"]["ECONT"].as<std::string>();
          //std::string fname_ROCs = params["configs"]["ROCs"].as<std::string>();
          LogDebug("HGCalConfigESSourceFromYAML") << "Found module id=" << std::hex << id
            << ", ECOND=" << fname_ECOND << ", ECONT=" << fname_ECONT;
          //parseECONConfigYAML(fname_ECON,cond);
          //parseROCConfigYAML(fname_ROCs,cond);
          uint8_t gain = (uint8_t) (gain_>=1 ? gain_ : 1); // manual override
          cond->moduleConfigs[0].gains[id+0*64] = gain; // ROC 0, half 0
          cond->moduleConfigs[0].gains[id+1*64] = gain; // ROC 0, half 1
          cond->moduleConfigs[0].gains[id+2*64] = gain; // ROC 1, half 0
          cond->moduleConfigs[0].gains[id+3*64] = gain; // ROC 1, half 1
          cond->moduleConfigs[0].gains[id+4*64] = gain; // ROC 2, half 0
          cond->moduleConfigs[0].gains[id+5*64] = gain; // ROC 2, half 1
        }
      } else {
        edm::LogWarning("HGCalConfigESSourceFromYAML")
            << "The YAML configuration is missing a 'ECONs' node. The conditions format may hence be invalid.\n"
            << filename;
      }
      
      // PARSE META DATA NODE: place holders
      cond->moduleConfigs[0].charMode = (bool) (charMode_>=0 ? charMode_ : 0); // manual override
      cond->moduleConfigs[0].injcalib = 0;
      cond->moduleConfigs[0].injgain = 0;
      LogDebug("HGCalConfigESSourceFromYAML") << "Placeholders: charMode=" << cond->moduleConfigs[0].charMode
        << ", gain=" << cond->moduleConfigs[0].gains[0] << ", injcalib=" << cond->moduleConfigs[0].injcalib
        << ", injgain=" << cond->moduleConfigs[0].injgain;
      
      //const auto yaml_file = YAML::LoadFile(filename);
      //if (const auto config = yaml_file["metaData"]; config.IsDefined()) {
      //  int charMode = yaml_file["metaData"]["characMode"].as<int>();
      //  assert(charMode==0 or charMode==1);
      //  cond->moduleConfigs[0].charMode = (bool) charMode;
      //
      //  cond->moduleConfigs[0].injcalib = 0;
      //  if(yaml_file["metaData"]["chip_params"]["Calib"])
      //    cond->moduleConfigs[0].injcalib = yaml_file["metaData"]["chip_params"]["Calib"].as<int>();
      //  cond->moduleConfigs[0].injgain = 0;
      //  if(yaml_file["metaData"]["chip_params"]["Inj_gain"])
      //    cond->moduleConfigs[0].injgain = yaml_file["metaData"]["chip_params"]["Inj_gain"].as<int>();
      //} else {
      //  edm::LogWarning("HGCalConfigESSourceFromYAML")
      //      << "The YAML configuration is missing a 'metaData' node. The conditions format may hence be invalid.\n"
      //      << filename;
      //}
      
    } catch (const YAML::BadFile& err) {
      throw cms::Exception("HGCalConfigESSourceFromYAML") << "Bad file error: " << err.msg;
    } catch (const YAML::ParserException& err) {
      throw cms::Exception("HGCalConfigESSourceFromYAML") << "Parser exception: " << err.msg;
    }
    return cond;
  }
  
};

DEFINE_FWK_EVENTSETUP_SOURCE(HGCalConfigESSourceFromYAML);
