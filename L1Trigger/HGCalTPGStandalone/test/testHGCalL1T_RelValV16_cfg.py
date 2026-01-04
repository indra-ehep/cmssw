import FWCore.ParameterSet.Config as cms
####################################################################
import os, sys, re
import FWCore.ParameterSet.VarParsing as VarParsing
####################################################################

from Configuration.Eras.Era_Phase2C17I13M9_cff import Phase2C17I13M9
process = cms.Process('DIGI',Phase2C17I13M9)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.Geometry.GeometryExtended2026D88Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2026D88_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedHLLHC14TeV_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.SimL1Emulator_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')


####################################################################
### SETUP OPTIONS
options = VarParsing.VarParsing('standard')
options.register('infile',
                 "root://eosuser.cern.ch///eos/cms/store/group/dpg_hgcal/comm_hgcal/TPG/geomv16_el7_sim/25603.0_SingleElectronPt1000+2026D100_N1000/step3.root",
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "input file to process: root://eosuser.cern.ch///$eos_path/step3.root or file:/$local_path/step3.root")

options.register('outfile',
                 "ntuple.root",
                  VarParsing.VarParsing.multiplicity.singleton,
                  VarParsing.VarParsing.varType.string,
                  "iterations 0 1 2 ....")

### get and parse the command line arguments
options.parseArguments()

print(options)

####################################################################

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# Input source
fileInput = options.infile #"root://eosuser.cern.ch///eos/cms/store/group/dpg_hgcal/comm_hgcal/TPG/geomv16_el7_sim/25603.0_SingleElectronPt1000+2026D100_N1000/step3.root"
print("Input file:    ", fileInput)
process.source = cms.Source("PoolSource",
       fileNames = cms.untracked.vstring(fileInput),
       inputCommands=cms.untracked.vstring(
           'keep *',
           )
        )
process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    version = cms.untracked.string('$Revision: 1.20 $'),
    annotation = cms.untracked.string('SingleElectronPt10_cfi nevts:10'),
    name = cms.untracked.string('Applications')
)

# Output definition
fileOutput = options.outfile
print("Output file:    ", fileOutput)
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string(fileOutput)
    )

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T21', '')

# load HGCAL TPG simulation
process.load('L1Trigger.HGCalTPGStandalone.hgcalTriggerPrimitives_cff')

process.hgcl1tpg_step = cms.Path(process.L1THGCalTriggerPrimitives)


# load ntuplizer
process.load('L1Trigger.HGCalTPGStandalone.hgcalTriggerNtuples_cff')
process.ntuple_step = cms.Path(process.L1THGCalTriggerNtuples)

# Schedule definition
process.schedule = cms.Schedule(process.hgcl1tpg_step, process.ntuple_step)

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion
