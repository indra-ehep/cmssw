import FWCore.ParameterSet.Config as cms

from L1Trigger.L1THGCal.l1tHGCalTriggerGeometryESProducer_cfi import *
from L1Trigger.HGCalTPGStandalone.l1tHGCalConcentratorProducer_cfi import *


L1THGCalConcentratorSL = cms.Task(l1tHGCalConcentratorProducerSL)
L1THGCalConcentratorHFNoseSL = cms.Task(l1tHGCalConcentratorProducerHFNoseSL)

