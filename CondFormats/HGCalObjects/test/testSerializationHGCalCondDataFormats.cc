#include "CondFormats/Serialization/interface/Test.h"
#include "CondFormats/HGCalObjects/src/headers.h"

int main()
{
  //generic configurables
  testSerialization<HGCalCondSerializableGenericConfig>();

  //si cell
  testSerialization<HGCalSiCellChannelInfo>();
  testSerialization<std::vector<HGCalSiCellChannelInfo>>();
  testSerialization<HGCalCondSerializableSiCellChannelInfo>();
  testSerialization<std::vector<HGCalCondSerializableSiCellChannelInfo>>();

  //sipm-on-tile cell
  testSerialization<HGCalSiPMTileInfo>();
  testSerialization<std::vector<HGCalSiPMTileInfo>>();
  testSerialization<HGCalCondSerializableSiPMTileInfo>();
  testSerialization<std::vector<HGCalCondSerializableSiPMTileInfo>>();

  //module info
  testSerialization<HGCalModuleInfo>();
  testSerialization<std::vector<HGCalModuleInfo>>();
  testSerialization<HGCalCondSerializableModuleInfo>();
  testSerialization<std::vector<HGCalCondSerializableModuleInfo>>();

  return 0;
}
