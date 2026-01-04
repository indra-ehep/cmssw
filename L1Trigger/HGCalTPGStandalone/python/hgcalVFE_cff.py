import FWCore.ParameterSet.Config as cms

from L1Trigger.L1THGCal.l1tHGCalTriggerGeometryESProducer_cfi import *
from L1Trigger.HGCalTPGStandalone.l1tHGCalVFEProducer_cfi import *

L1THGCalVFESL = cms.Task(l1tHGCalVFEProducerSL)
L1THFnoseVFESL = cms.Task(l1tHFnoseVFEProducerSL)

