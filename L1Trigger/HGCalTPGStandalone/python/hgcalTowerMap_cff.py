import FWCore.ParameterSet.Config as cms

from L1Trigger.L1THGCal.l1tHGCalTriggerGeometryESProducer_cfi import *
from L1Trigger.HGCalTPGStandalone.l1tHGCalTowerMapProducer_cfi import *


L1THGCalTowerMap = cms.Task(l1tHGCalTowerMapProducerSL)

L1THGCalTowerMapHFNose = cms.Task(l1tHGCalTowerMapProducerHFNoseSL)

