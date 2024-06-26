#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DQMOffline/PFTau/interface/Matchers.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Candidate/interface/CandMatchMap.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <algorithm>
#include <numeric>
#include <regex>
#include <sstream>
#include <vector>
#include <memory>

class PFJetAnalyzerDQM : public DQMEDAnalyzer {
public:
  PFJetAnalyzerDQM(const edm::ParameterSet&);
  void analyze(const edm::Event&, const edm::EventSetup&) override;

protected:
  //Book histograms
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;

private:
  class Plot1DInBin {
  public:
    const std::string name, title;
    const uint32_t nbins;
    const float min, max;
    const float ptbin_low, ptbin_high, etabin_low, etabin_high;
    MonitorElement* plot_;

    Plot1DInBin(const std::string _name,
                const std::string _title,
                const uint32_t _nbins,
                const float _min,
                const float _max,
                float _ptbin_low,
                float _ptbin_high,
                float _etabin_low,
                float _etabin_high)
        : name(_name),
          title(_title),
          nbins(_nbins),
          min(_min),
          max(_max),
          ptbin_low(_ptbin_low),
          ptbin_high(_ptbin_high),
          etabin_low(_etabin_low),
          etabin_high(_etabin_high) {}

    void book(DQMStore::IBooker& booker) { plot_ = booker.book1D(name, title, nbins, min, max); }

    void fill(float value) {
      assert(plot_ != nullptr);
      plot_->Fill(value);
    }

    //Check if a jet with a value v would be in the bin that applies to this plot
    bool isInBin(float v, float low, float high) { return v >= low && v < high; }

    bool isInPtBin(float pt) { return isInBin(pt, ptbin_low, ptbin_high); }

    bool isInEtaBin(float eta) { return isInBin(eta, etabin_low, etabin_high); }

    bool isInPtEtaBin(float pt, float eta) { return isInPtBin(pt) && isInEtaBin(eta); }
  };

  class Plot1DInBinVariable {
  public:
    const std::string name, title;
    std::unique_ptr<TH1F> base_hist;
    const float ptbin_low, ptbin_high, etabin_low, etabin_high;
    MonitorElement* plot_;

    Plot1DInBinVariable(const std::string _name,
                        const std::string _title,
                        std::unique_ptr<TH1F> _base_hist,
                        float _ptbin_low,
                        float _ptbin_high,
                        float _etabin_low,
                        float _etabin_high)
        : name(_name),
          title(_title),
          base_hist(std::move(_base_hist)),
          ptbin_low(_ptbin_low),
          ptbin_high(_ptbin_high),
          etabin_low(_etabin_low),
          etabin_high(_etabin_high) {}

    void book(DQMStore::IBooker& booker) { plot_ = booker.book1D(name.c_str(), base_hist.get()); }

    void fill(float value) {
      assert(plot_ != nullptr);
      plot_->Fill(value);
    }

    //Check if a jet with a value v would be in the bin that applies to this plot
    bool isInBin(float v, float low, float high) { return v >= low && v < high; }

    bool isInPtBin(float pt) { return isInBin(pt, ptbin_low, ptbin_high); }

    bool isInEtaBin(float eta) { return isInBin(eta, etabin_low, etabin_high); }

    bool isInPtEtaBin(float pt, float eta) { return isInPtBin(pt) && isInEtaBin(eta); }
  };

  std::vector<Plot1DInBin> jetResponsePlots;
  std::vector<Plot1DInBin> jetResponsePlots_noJEC;
  std::vector<Plot1DInBinVariable> genJetPlots;
  std::vector<Plot1DInBinVariable> genJetPlots_matched;
  std::vector<Plot1DInBinVariable> genJetPlots_unmatched;
  std::vector<Plot1DInBinVariable> recoJetPlots;
  std::vector<Plot1DInBinVariable> recoJetPlots_matched;
  std::vector<Plot1DInBinVariable> recoJetPlots_unmatched;
  // Is this data or MC?
  bool isMC;

  float jetDeltaR;

  bool genJetsOn, recoJetsOn;

  std::string jetCollectionName;

  edm::InputTag recoJetsLabel;
  edm::InputTag genJetsLabel;
  edm::EDGetTokenT<edm::View<pat::Jet>> recoJetsToken;
  edm::EDGetTokenT<edm::View<reco::Jet>> genJetsToken;
  edm::EDGetTokenT<reco::CandViewMatchMap> srcRefToJetMap;

  void fillJetResponse(edm::View<pat::Jet>& recoJetCollection, edm::View<reco::Jet>& genJetCollection);
  void prepareJetResponsePlots(const std::vector<edm::ParameterSet>& genjet_plots_pset);
  void prepareGenJetPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset);
  void prepareGenJetMatchedPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset);
  void prepareGenJetUnmatchedPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset);
  void prepareRecoJetPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset);
  void prepareRecoJetMatchedPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset);
  void prepareRecoJetUnmatchedPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset);
};

void PFJetAnalyzerDQM::prepareJetResponsePlots(const std::vector<edm::ParameterSet>& response_plots) {
  for (auto& pset : response_plots) {
    //Low and high edges of the pt and eta bins for jets to pass to be filled into this histogram
    const auto ptbin_low = pset.getParameter<double>("ptBinLow");
    const auto ptbin_high = pset.getParameter<double>("ptBinHigh");
    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    const auto response_nbins = pset.getParameter<uint32_t>("responseNbins");
    const auto response_low = pset.getParameter<double>("responseLow");
    const auto response_high = pset.getParameter<double>("responseHigh");

    const auto name = pset.getParameter<std::string>("name");
    const auto title = pset.getParameter<std::string>("title");

    // for title of raw jet response histograms
    auto rawTitle = title;
    rawTitle = rawTitle.replace(rawTitle.begin(), rawTitle.begin(), "Raw ");

    jetResponsePlots.push_back(Plot1DInBin(
        name, title, response_nbins, response_low, response_high, ptbin_low, ptbin_high, etabin_low, etabin_high));

    jetResponsePlots_noJEC.push_back(Plot1DInBin(
        name, rawTitle, response_nbins, response_low, response_high, ptbin_low, ptbin_high, etabin_low, etabin_high));
  }
  if (jetResponsePlots.size() > 200) {
    throw std::runtime_error("Requested too many jet response plots, aborting as this seems unusual.");
  }
}

void PFJetAnalyzerDQM::prepareGenJetPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset) {
  for (auto& pset : genjet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name");
    const auto title = pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    genJetPlots.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

void PFJetAnalyzerDQM::prepareGenJetMatchedPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset) {
  for (auto& pset : genjet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name") + "_matched";
    const auto title = "Matched " + pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    genJetPlots_matched.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

void PFJetAnalyzerDQM::prepareGenJetUnmatchedPlots(const std::vector<edm::ParameterSet>& genjet_plots_pset) {
  for (auto& pset : genjet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name") + "_unmatched";
    const auto title = "Unmatched " + pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    genJetPlots_unmatched.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

void PFJetAnalyzerDQM::prepareRecoJetPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset) {
  for (auto& pset : recojet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name");
    const auto title = pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    recoJetPlots.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

void PFJetAnalyzerDQM::prepareRecoJetMatchedPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset) {
  for (auto& pset : recojet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name") + "_matched";
    const auto title = "Matched " + pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    recoJetPlots_matched.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

void PFJetAnalyzerDQM::prepareRecoJetUnmatchedPlots(const std::vector<edm::ParameterSet>& recojet_plots_pset) {
  for (auto& pset : recojet_plots_pset) {
    const auto name = pset.getParameter<std::string>("name") + "_unmatched";
    const auto title = "Unmatched " + pset.getParameter<std::string>("title");

    //Low and high edges of the eta bins for jets to pass to be filled into this histogram
    const auto ptbins_d = pset.getParameter<std::vector<double>>("ptBins");
    std::vector<float> ptbins(ptbins_d.begin(), ptbins_d.end());

    const auto etabin_low = pset.getParameter<double>("etaBinLow");
    const auto etabin_high = pset.getParameter<double>("etaBinHigh");

    recoJetPlots_unmatched.push_back(Plot1DInBinVariable(
        name,
        title,
        std::make_unique<TH1F>(name.c_str(), title.c_str(), static_cast<int>(ptbins.size()) - 1, ptbins.data()),
        0.0,
        0.0,
        etabin_low,
        etabin_high));
  }
}

PFJetAnalyzerDQM::PFJetAnalyzerDQM(const edm::ParameterSet& iConfig) {
  recoJetsLabel = iConfig.getParameter<edm::InputTag>("recoJetCollection");
  genJetsLabel = iConfig.getParameter<edm::InputTag>("genJetCollection");

  //label for making new folder
  jetCollectionName = recoJetsLabel.label();

  //DeltaR for reco to gen jet matching
  jetDeltaR = iConfig.getParameter<double>("jetDeltaR");

  //for turn genJet on/off
  genJetsOn = iConfig.getParameter<bool>("genJetsOn");
  recoJetsOn = iConfig.getParameter<bool>("recoJetsOn");

  //Create all jet response plots in bins of genjet pt and eta
  const auto& response_plots = iConfig.getParameter<std::vector<edm::ParameterSet>>("responsePlots");
  prepareJetResponsePlots(response_plots);

  const auto& genjet_plots = iConfig.getParameter<std::vector<edm::ParameterSet>>("genJetPlots");
  prepareGenJetPlots(genjet_plots);
  prepareGenJetMatchedPlots(genjet_plots);
  prepareGenJetUnmatchedPlots(genjet_plots);

  const auto& recojet_plots = iConfig.getParameter<std::vector<edm::ParameterSet>>("recoJetPlots");
  prepareRecoJetPlots(recojet_plots);
  prepareRecoJetMatchedPlots(recojet_plots);
  prepareRecoJetUnmatchedPlots(recojet_plots);

  recoJetsToken = consumes<edm::View<pat::Jet>>(recoJetsLabel);
  genJetsToken = consumes<edm::View<reco::Jet>>(genJetsLabel);
}

void PFJetAnalyzerDQM::fillJetResponse(edm::View<pat::Jet>& recoJetCollection, edm::View<reco::Jet>& genJetCollection) {
  //match gen jets to reco jets, require minimum jetDeltaR, choose closest, do not try to match charge
  std::vector<int> matchIndices;
  std::vector<int> matchIndicesReco;
  PFB::match(genJetCollection, recoJetCollection, matchIndices, false, jetDeltaR);
  PFB::match(recoJetCollection, genJetCollection, matchIndicesReco, false, jetDeltaR);

  //Fill recojet pt if recoJetOn
  for (unsigned int i = 0; i < recoJetCollection.size(); i++) {
    const auto& recoJet = recoJetCollection.at(i);
    const auto pt_reco = recoJet.pt();
    const auto eta_reco = abs(recoJet.eta());
    const int iMatch_reco = matchIndicesReco[i];
    if (recoJetsOn) {
      for (auto& plot : recoJetPlots) {
        if (plot.isInEtaBin(eta_reco)) {
          plot.fill(pt_reco);
        }
      }
      if (iMatch_reco != -1) {
        for (auto& plot : recoJetPlots_matched) {
          if (plot.isInEtaBin(eta_reco)) {
            plot.fill(pt_reco);
          }
        }
      } else {
        for (auto& plot : recoJetPlots_unmatched) {
          if (plot.isInEtaBin(eta_reco)) {
            plot.fill(pt_reco);
          }
        }
      }
    }
  }

  for (unsigned int i = 0; i < genJetCollection.size(); i++) {
    const auto& genJet = genJetCollection.at(i);
    const auto pt_gen = genJet.pt();
    const auto eta_gen = abs(genJet.eta());
    const int iMatch = matchIndices[i];

    //Fill genjet pt if genJetOn
    if (genJetsOn) {
      for (auto& plot : genJetPlots) {
        if (plot.isInEtaBin(eta_gen)) {
          plot.fill(pt_gen);
        }
      }
    }
    if (recoJetsOn) {
      if (iMatch != -1) {
        for (auto& plot : genJetPlots_matched) {
          if (plot.isInEtaBin(eta_gen)) {
            plot.fill(pt_gen);
          }
        }
      } else {
        for (auto& plot : genJetPlots_unmatched) {
          if (plot.isInEtaBin(eta_gen)) {
            plot.fill(pt_gen);
          }
        }
      }
    }

    //If gen jet had a matched reco jet
    if (iMatch != -1) {
      const auto& recoJet = recoJetCollection[iMatch];
      auto pt_reco = recoJet.pt();

      const auto response = pt_reco / pt_gen;
      const auto response_raw = pt_reco * recoJet.jecFactor("Uncorrected") / pt_gen;

      //Loop linearly through all plots and check if they match the pt and eta bin
      //this is not algorithmically optimal but we don't expect to more than a few hundred plots
      //If this turns out to be a problem, can easily make a 2D-map for indices
      for (auto& plot : jetResponsePlots) {
        if (plot.isInPtEtaBin(pt_gen, eta_gen)) {
          plot.fill(response);
        }
      }
      // this loop should be for NoJEC plots
      for (auto& plot : jetResponsePlots_noJEC) {
        if (plot.isInPtEtaBin(pt_gen, eta_gen)) {
          plot.fill(response_raw);
        }
      }
    }
  }
}

void PFJetAnalyzerDQM::bookHistograms(DQMStore::IBooker& booker, edm::Run const&, edm::EventSetup const&) {
  booker.setCurrentFolder("ParticleFlow/JetResponse/" + jetCollectionName + "/JEC/");
  for (auto& plot : jetResponsePlots) {
    plot.book(booker);
  }
  //Book plots for noJEC
  booker.setCurrentFolder("ParticleFlow/JetResponse/" + jetCollectionName + "/noJEC/");
  for (auto& plot : jetResponsePlots_noJEC) {
    plot.book(booker);
  }

  if (recoJetsOn) {
    booker.setCurrentFolder("ParticleFlow/JetResponse/" + jetCollectionName + "/noJEC/");
    for (auto& plot : genJetPlots_matched) {
      plot.book(booker);
    }
    for (auto& plot : genJetPlots_unmatched) {
      plot.book(booker);
    }
    booker.setCurrentFolder("ParticleFlow/JetResponse/" + jetCollectionName + "/JEC/");
    for (auto& plot : recoJetPlots) {
      plot.book(booker);
    }
    for (auto& plot : recoJetPlots_matched) {
      plot.book(booker);
    }
    for (auto& plot : recoJetPlots_unmatched) {
      plot.book(booker);
    }
  }

  //Book plots for gen-jet pt spectra
  if (genJetsOn) {
    booker.setCurrentFolder("ParticleFlow/GenJets/");
    for (auto& plot : genJetPlots) {
      plot.book(booker);
    }
  }
}

void PFJetAnalyzerDQM::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  edm::Handle<edm::View<pat::Jet>> recoJetCollectionHandle;
  iEvent.getByToken(recoJetsToken, recoJetCollectionHandle);
  auto recoJetCollection = *recoJetCollectionHandle;

  isMC = !iEvent.isRealData();

  if (isMC) {
    edm::Handle<edm::View<reco::Jet>> genJetCollectionHandle;
    iEvent.getByToken(genJetsToken, genJetCollectionHandle);
    auto genJetCollection = *genJetCollectionHandle;

    fillJetResponse(recoJetCollection, genJetCollection);
  }
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PFJetAnalyzerDQM);
