// -*- C++ -*-
//
// Package:    L1TBMTFConverter
// Class:      L1TBMTFConverter
//
/**\class L1TBMTFConverter L1TBMTFConverter.cc L1Trigger/L1TGlobalMuon/plugins/L1TBMTFConverter.cc

 Description: Takes txt-file input and produces barrel- / overlap- / forward TF muons

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Joschka Philip Lingemann,40 3-B01,+41227671598,
//         Created:  Thu Oct  3 10:12:30 CEST 2013
// $Id$
//
//

// system include files
#include <memory>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"

#include <iostream>
//
// class declaration
//
using namespace l1t;

class L1TBMTFConverter : public edm::stream::EDProducer<> {
public:
  explicit L1TBMTFConverter(const edm::ParameterSet&);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<RegionalMuonCandBxCollection> m_barrelTfInputToken;
  edm::InputTag m_barrelTfInputTag;
  std::map<int, int> ptMap_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
L1TBMTFConverter::L1TBMTFConverter(const edm::ParameterSet& iConfig) {
  m_barrelTfInputTag = iConfig.getParameter<edm::InputTag>("barrelTFInput");
  m_barrelTfInputToken = consumes<RegionalMuonCandBxCollection>(m_barrelTfInputTag);
  //register your products
  produces<RegionalMuonCandBxCollection>("ConvBMTFMuons");
  ptMap_[0] = 0;
  ptMap_[1] = 0;
  ptMap_[2] = 3;
  ptMap_[3] = 4;
  ptMap_[4] = 5;
  ptMap_[5] = 6;
  ptMap_[6] = 7;
  ptMap_[7] = 8;
  ptMap_[8] = 9;
  ptMap_[9] = 10;
  ptMap_[10] = 12;
  ptMap_[11] = 14;
  ptMap_[12] = 16;
  ptMap_[13] = 20;
  ptMap_[14] = 24;
  ptMap_[15] = 28;
  ptMap_[16] = 32;
  ptMap_[17] = 36;
  ptMap_[18] = 40;
  ptMap_[19] = 50;
  ptMap_[20] = 60;
  ptMap_[21] = 70;
  ptMap_[22] = 80;
  ptMap_[23] = 90;
  ptMap_[24] = 100;
  ptMap_[25] = 120;
  ptMap_[26] = 140;
  ptMap_[27] = 160;
  ptMap_[28] = 180;
  ptMap_[29] = 200;
  ptMap_[30] = 240;
  ptMap_[31] = 280;
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void L1TBMTFConverter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  std::unique_ptr<RegionalMuonCandBxCollection> convMuons(new RegionalMuonCandBxCollection());

  Handle<RegionalMuonCandBxCollection> bmtfMuons;
  iEvent.getByToken(m_barrelTfInputToken, bmtfMuons);
  for (auto mu = bmtfMuons->begin(0); mu != bmtfMuons->end(0); ++mu) {
    RegionalMuonCand convMu((*mu));
    // int convPt = ptMap_.at(mu->hwPt());
    // int convPhi = (mu->hwPhi() * 4) - (mu->processor() * 48);
    // int convEta = getSigned(mu->hwEta())*3.54;
    int convEta = (mu->hwEta() - 32) * 3.54;
    // convMu.setHwPt(convPt);
    // convMu.setHwPhi(convPhi);
    convMu.setHwEta(convEta);
    // convMu.setTFIdentifiers(mu->processor()+1, mu->trackFinderType());
    convMuons->push_back(0, convMu);
  }

  iEvent.put(std::move(convMuons), "ConvBMTFMuons");
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void L1TBMTFConverter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TBMTFConverter);
