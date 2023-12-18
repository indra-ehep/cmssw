import FWCore.ParameterSet.Config as cms
from L1Trigger.L1THGCal.l1tHGCalBackEndLayer1Producer_cfi import layer1truncation_proc
from L1Trigger.L1THGCal.l1tHGCalBackEndLayer1Producer_cfi import stage1truncation_proc
from L1Trigger.L1THGCal.l1tHGCalBackEndLayer1Producer_cfi import truncation_params
from L1Trigger.L1THGCal.hgcalBackendLayer2_fwClustering_cfi import layer2ClusteringFw_Params

def custom_layer1_truncation(process):
    parameters = layer1truncation_proc.clone()
    process.l1tHGCalBackEndLayer1Producer.ProcessorParameters = parameters
    process.l1tHGCalBackEndLayer2Producer.InputCluster = cms.InputTag('l1tHGCalBackEndLayer1Producer:HGCalBackendLayer1Processor')
    process.l1tHGCalTowerProducer.InputTriggerCells = cms.InputTag('l1tHGCalBackEndLayer1Producer:HGCalBackendLayer1Processor')
    return process

def custom_stage1_truncation(process):
    parameters = stage1truncation_proc.clone()
    process.l1tHGCalBackEndLayer1Producer.ProcessorParameters = parameters
    process.l1tHGCalBackEndLayer2Producer.InputCluster = cms.InputTag('l1tHGCalBackEndStage1Producer:HGCalBackendStage1Processor')
    process.l1tHGCalTowerProducer.InputTriggerCells = cms.InputTag('l1tHGCalBackEndStage1Producer:HGCalBackendStage1Processor')
    return process
    
def custom_clustering_standalone(process):
    process.l1tHGCalBackEndLayer2Producer.ProcessorParameters.ProcessorName = cms.string('HGCalBackendLayer2Processor3DClusteringSA')
    process.l1tHGCalBackEndLayer2Producer.ProcessorParameters.DistributionParameters = truncation_params
    process.l1tHGCalBackEndLayer2Producer.ProcessorParameters.C3d_parameters.histoMax_C3d_clustering_parameters.layer2FwClusteringParameters = layer2ClusteringFw_Params
    return process
    
def custom_tower_standalone(process):
    process.l1tHGCalTowerProducer.ProcessorParameters.ProcessorName = cms.string('HGCalTowerProcessorSA')
    return process
    
def custom_conc_standalone(process):
    process.l1tHGCalConcentratorProducer.ProcessorParameters.ProcessorName = cms.string('HGCalConcentratorProcessorSelectionSA')
    process.l1tHGCalTowerMapProducer.InputTriggerSums = cms.InputTag('l1tHGCalConcentratorProducer:HGCalConcentratorProcessorSelectionSA')
    process.l1tHGCalTowerMapProducerHFNose.InputTriggerSums = cms.InputTag('l1tHGCalConcentratorProducer:HGCalConcentratorProcessorSelectionSA')
    process.l1tHGCalBackEndLayer1Producer.InputTriggerCells = cms.InputTag('l1tHGCalConcentratorProducer:HGCalConcentratorProcessorSelectionSA')
    process.l1tHGCalBackEndStage1Producer.InputTriggerCells = cms.InputTag('l1tHGCalConcentratorProducer:HGCalConcentratorProcessorSelectionSA')
    process.l1tHGCalBackEndLayer1ProducerHFNose.InputTriggerCells = cms.InputTag('l1tHGCalConcentratorProducer:HGCalConcentratorProcessorSelectionSA')
    return process
    
def custom_vfe_standalone(process):
    process.l1tHGCalVFEProducer.ProcessorParameters.ProcessorName = cms.string('HGCalVFEProcessorSumsSA')
    process.l1tHGCalConcentratorProducer.InputTriggerCells = cms.InputTag('l1tHGCalVFEProducer:HGCalVFEProcessorSumsSA')
    process.l1tHGCalConcentratorProducer.InputTriggerSums = cms.InputTag('l1tHGCalVFEProducer:HGCalVFEProcessorSumsSA')
    # process.l1tHGCalVFEProducer.ProcessorParameters.summationCfg.noiseSilicon.values =  cms.vdouble(0,0,0)
    # process.l1tHGCalVFEProducer.ProcessorParameters.summationCfg.noiseScintillator.noise_MIP = cms.double(0)
    # process.l1tHGCalVFEProducer.ProcessorParameters.summationCfg.noiseThreshold = cms.double(0)
    return process
    

