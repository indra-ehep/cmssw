#! /usr/bin/env cmsRun
import FWCore.ParameterSet.Config as cms
process = cms.Process("Calib")

# argument parser
from FWCore.ParameterSet.VarParsing import VarParsing
#infname = '/eos/cms/store/group/dpg_hgcal/tb_hgcal/2023/labtest/module822/pedestal_run0.yaml' # default
infname = 'test_hgcal_yamlmapper.yaml' # test mapper
options = VarParsing('python')
options.register('inputFile',infname,mytype=VarParsing.varType.string,info="Path to input file")
options.register('verb',1,mytype=VarParsing.varType.int,info="Verbosity level, 0=normal, 1=debug (default)")
options.parseArguments()
infname = options.inputFile
verb    = options.verb
print(f">>> inputFile={infname}")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = "DEBUG"
process.MessageLogger.debugModules = ["*"]

process.source = cms.Source('EmptyIOVSource',
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    lastValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.load('CalibCalorimetry.HGCalPlugins.hgCalConfigESSourceFromYAML_cfi')
process.hgCalConfigESSourceFromYAML.filename = infname
process.analyzer = cms.EDAnalyzer("HGCalConfigESSourceFromYAMLAnalyzer",
                                  label = cms.string(''))

process.source = cms.Source('EmptySource')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.p = cms.Path(
    process.analyzer
)
