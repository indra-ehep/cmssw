import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Phase2C11M9_cff import Phase2C11M9

process = cms.Process('gemTester',Phase2C11M9)

process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('EventFilter.GEMRawToDigi.GEMPackingTester_cfi')
process.load('Configuration.EventContent.EventContent_cff')

process.GEMPackingTester.gemDigi = cms.InputTag("muonGEMDigis",'','gemTester')
process.gemPacker.useDBEMap = False
process.muonGEMDigis.useDBEMap = False
process.muonGEMDigis.keepDAQStatus = True
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.source = cms.Source("PoolSource",                           
  fileNames = cms.untracked.vstring('file:/store/relval/CMSSW_11_3_0_pre4/RelValZMM_14/GEN-SIM-RECO/PU_113X_mcRun4_realistic_v4_2026D76PU200-v1/00000/028001e8-5c24-48e7-8162-5da736ad7d38.root'),
)

process.FEVTDEBUGHLToutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM-RECO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:muonGEMDigis.root'),
    outputCommands = cms.untracked.vstring( ('drop *', 'keep *_muonGEMDigis_*_*')),
    splitLevel = cms.untracked.int32(0)
)
process.TFileService = cms.Service('TFileService', fileName = cms.string('gemTester.root') )
process.rawDataCollector.RawCollectionList = cms.VInputTag(cms.InputTag("gemPacker",'','gemTester'))
process.MessageLogger.cerr.threshold = "DEBUG"
#process.MessageLogger.debugModules = ["gemPacker", "muonGEMDigis"]
process.MessageLogger.debugModules = ["muonGEMDigis"]

process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)

process.p = cms.Path(process.gemPacker+process.rawDataCollector+process.muonGEMDigis+process.GEMPackingTester)

