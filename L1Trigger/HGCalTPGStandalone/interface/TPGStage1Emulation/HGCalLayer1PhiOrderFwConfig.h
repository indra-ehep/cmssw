#ifndef __HGCalLayer1PhiOrderFwConfig_h__
#define __HGCalLayer1PhiOrderFwConfig_h__

#include <vector>
#include <cstdint>  // uint32_t
#include <unordered_map>

namespace l1thgcfirmware {

  class HGCalLayer1PhiOrderFwConfig {
  public:
    HGCalLayer1PhiOrderFwConfig() {}

    void setSector120(const unsigned sector) { sector120_ = sector; }
    void setFPGAID(const uint32_t fpga_id) { fpga_id_ = fpga_id; }

    /*void
    configureMappingInfo() {  //creates a map between column number and a vector of pairs containing the available (channel,frame) combinations. Each frame accepts 4 TCs, so each (channel,frame) combination appears 4 times in the vector. We could optionally add a unique "slot" index to this vector, but it is not strictly necessary. The function also creates a vector of pairs between column number and number of TCs the column can accept.
      for (unsigned i = 0; i < 20; ++i) {
        //std::pair<unsigned, int> mod_col_pair = std::make_pair(dummyModId_,i);//
        for (unsigned j = 0; j < 2; ++j) {
          for (unsigned k = 0; k < 4; ++k) {  //number of slots per frame
            chn_frame_slots_per_mod_and_col_[dummyModId_][i].push_back(std::make_pair(i + 3, j));
          }
          for (unsigned k = 0; k < 4; ++k) {  //number of slots per frame
            chn_frame_slots_per_mod_and_col_[dummyModId_][i].push_back(std::make_pair(i + 4, j));
          }
        }
        max_tcs_per_module_and_column_[dummyModId_].push_back(
            std::make_pair(i, chn_frame_slots_per_mod_and_col_[dummyModId_][i].size()));
      }
    }*/

    void configureTestSetupMappingInfo() {
      for (unsigned iModGroup = 0; iModGroup < 60; iModGroup++) {
        for (unsigned j = 0; j < 4; j++) {  //BC low (mod 1) - 4 bins, 1TC/bin
          chn_frame_slots_per_mod_and_col_[1 + iModGroup * 5][j].push_back(std::make_pair(0, 0));
          max_tcs_per_module_and_column_[1 + iModGroup * 5].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[1][j].size()));
        }
        for (unsigned j = 0; j < 7; j++) {  //BC high (mod 0) - 7 bins, 2 TC/bin bin 1-2; 1TC/bin rest
          chn_frame_slots_per_mod_and_col_[iModGroup * 5][j].push_back(std::make_pair(0, 0));
          if (j < 2) {
            chn_frame_slots_per_mod_and_col_[iModGroup * 5][j].push_back(std::make_pair(0, 0));
          }
          max_tcs_per_module_and_column_[iModGroup * 5].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[0][j].size()));
        }
        for (unsigned j = 0; j < 3; j++) {  //STC16 (mod 3-4) - 3 bins, 1TC/bin
          for (unsigned i = 3; i < 5; i++) {
            chn_frame_slots_per_mod_and_col_[i + iModGroup * 5][j].push_back(std::make_pair(0, 0));
            max_tcs_per_module_and_column_[i + iModGroup * 5].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[i][j].size()));
          }
        }
        for (unsigned j = 0; j < 7; j++) {  //STC4 (mod 2) - 7 bins, 2TC/bin bin 1-5; 1TC/bin rest
          chn_frame_slots_per_mod_and_col_[2 + iModGroup * 5][j].push_back(std::make_pair(0, 0));
          if (j < 5) {
            chn_frame_slots_per_mod_and_col_[2 + iModGroup * 5][j].push_back(std::make_pair(0, 0));
          }
          max_tcs_per_module_and_column_[2 + iModGroup * 5].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[2][j].size()));
        }
      }
    }

    void configureTestBeamMappingInfo() {
      for (unsigned j = 0; j < 7; j++) {  //Module 256, 8448 - BC (high occ for now)
        chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0, 0));
        if (j < 2) {
          chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0, 0));
        }
        max_tcs_per_module_and_column_[256].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[256][j].size()));
        max_tcs_per_module_and_column_[8448].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[8448][j].size()));
      }

      for (unsigned j = 0; j < 7; j++) {  // Module 768,8960 - STC4A
        chn_frame_slots_per_mod_and_col_[768][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[8960][j].push_back(std::make_pair(0, 0));
        if (j < 5) {
          chn_frame_slots_per_mod_and_col_[768][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[8960][j].push_back(std::make_pair(0, 0));
        }
        max_tcs_per_module_and_column_[768].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[768][j].size()));
        max_tcs_per_module_and_column_[8960].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[8960][j].size()));
      }

      for (unsigned j = 0; j < 3; j++) {  // Module 1280,9472 - CTC4A in Jul TB
        chn_frame_slots_per_mod_and_col_[1280][j].push_back(std::make_pair(0, 0));
        max_tcs_per_module_and_column_[1280].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[1280][j].size()));
        chn_frame_slots_per_mod_and_col_[9472][j].push_back(std::make_pair(0, 0));
        max_tcs_per_module_and_column_[9472].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[9472][j].size()));
      }
    }

    void configureSeptemberTestBeamMappingInfo() {
      for (unsigned j = 0; j < 9;
           j++) {  //Module 256 - BC, 9 bins with first two having 2 slots, Module 768 - BC, 9 bins, Module 1280 - BC, 9 bins, 5 with 2 TC/bin, 8448 - STC16, 9 bins with first first 5 bins having two slots; and 8960 - STC16, 9 bins. Treat 9472 similarly, even if not read out
                   //In the third train: 16640 - STC4, as 256; Module 17152, as 768. Module 24832: as 1280. Module 25344: as 8448
        chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[768][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[1280][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[8960][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[9472][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[16640][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[17152][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[24832][j].push_back(std::make_pair(0, 0));
        chn_frame_slots_per_mod_and_col_[25344][j].push_back(std::make_pair(0, 0));
        if (j < 2) {
          chn_frame_slots_per_mod_and_col_[256][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[16640][j].push_back(std::make_pair(0, 0));
        }
        if (j < 5) {
          chn_frame_slots_per_mod_and_col_[1280][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[24832][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[8448][j].push_back(std::make_pair(0, 0));
          chn_frame_slots_per_mod_and_col_[25344][j].push_back(std::make_pair(0, 0));
        }
        max_tcs_per_module_and_column_[256].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[256][j].size()));
        max_tcs_per_module_and_column_[768].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[768][j].size()));
        max_tcs_per_module_and_column_[1280].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[1280][j].size()));
        max_tcs_per_module_and_column_[8448].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[8448][j].size()));
        max_tcs_per_module_and_column_[8960].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[8960][j].size()));
        max_tcs_per_module_and_column_[9472].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[9472][j].size()));
        max_tcs_per_module_and_column_[16640].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[16640][j].size()));
        max_tcs_per_module_and_column_[17152].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[17152][j].size()));
        max_tcs_per_module_and_column_[24832].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[24832][j].size()));
        max_tcs_per_module_and_column_[25344].push_back(std::make_pair(j, chn_frame_slots_per_mod_and_col_[25344][j].size()));
      }
    }

    void configureSeptemberTBTDAQReadoutInfo() {
      for (unsigned j = 0; j < 9; j++) {
        //Module 256: 9 bins, 2 slots for the first two. 768: 4 bins, 1 slot/bin. 1280: 4 bins, 1 slot/bin. 8448: 9 bins, 2 slots for the first two. 8960: 9 bins. 9472: 0 bins. 16640: 8 bins in TDAQ output, 2 slots for the first 2. 17152: 4 bins. 24832 : 4 bins. 25344 : 9 bins, 2 slots for the first 2
        tdaq_slots_per_bin_per_mod_[256].push_back(std::make_pair(j, 0));  //This means bin j, slot 0.
        tdaq_slots_per_bin_per_mod_[8448].push_back(std::make_pair(j, 0));
        tdaq_slots_per_bin_per_mod_[8960].push_back(std::make_pair(j, 0));
        tdaq_slots_per_bin_per_mod_[25344].push_back(std::make_pair(j, 0));
        if (j < 2) {
          tdaq_slots_per_bin_per_mod_[256].push_back(std::make_pair(j, 1));
          tdaq_slots_per_bin_per_mod_[8448].push_back(std::make_pair(j, 1));
          tdaq_slots_per_bin_per_mod_[16640].push_back(std::make_pair(j, 1));
          tdaq_slots_per_bin_per_mod_[25344].push_back(std::make_pair(j, 1));
        }
        if (j < 4) {
          tdaq_slots_per_bin_per_mod_[768].push_back(std::make_pair(j, 0));
          tdaq_slots_per_bin_per_mod_[1280].push_back(std::make_pair(j, 0));
          tdaq_slots_per_bin_per_mod_[17152].push_back(std::make_pair(j, 0));
          tdaq_slots_per_bin_per_mod_[24832].push_back(std::make_pair(j, 0));
        }
        if (j < 8) {  //Only to match the observed output where the last TC that is supposed to be read out is missing. In reality 9 TCs read out.
          tdaq_slots_per_bin_per_mod_[16640].push_back(std::make_pair(j, 0));
        }
      }
    }

    void configureTestSetupTDAQReadoutInfo() {
      for (unsigned iModGroup = 0; iModGroup < 60; iModGroup++) {
        for (unsigned j = 0; j < 4; j++) {  //BC low (mod 1) - 4 bins, 1TC/bin
          tdaq_slots_per_bin_per_mod_[1 + iModGroup * 5].push_back(std::make_pair(j, 0));
        }
        for (unsigned j = 0; j < 7; j++) {  //BC high (mod 0) - 7 bins, 2 TC/bin bin 1-2; 1TC/bin rest
          tdaq_slots_per_bin_per_mod_[iModGroup * 5].push_back(std::make_pair(j, 0));
          if (j < 2) {
            tdaq_slots_per_bin_per_mod_[iModGroup * 5].push_back(std::make_pair(j, 1));
          }
        }
        for (unsigned j = 0; j < 3; j++) {  //STC16 (mod 3-4) - 3 bins, 1TC/bin
          for (unsigned i = 3; i < 5; i++) {
            tdaq_slots_per_bin_per_mod_[i + iModGroup * 5].push_back(std::make_pair(j, 0));
          }
        }
        for (unsigned j = 0; j < 4; j++) {  //STC4 (mod 2) - 4 bins, 1TC/bin (NB this only retrieves a part of the TCs that are in the input)
          tdaq_slots_per_bin_per_mod_[2 + iModGroup * 5].push_back(std::make_pair(j, 0));
        }
      } 
    }

    unsigned phiSector() const { return sector120_; }
    uint32_t fpgaID() const { return fpga_id_; }
    bool modIsConfigured(unsigned moduleId) const { return max_tcs_per_module_and_column_.count(moduleId); }
    unsigned getNumberOfColumns(unsigned moduleId) const { return max_tcs_per_module_and_column_.at(moduleId).size(); }
    unsigned getColBudgetAtIndex(unsigned moduleId, unsigned theColumnIndex) const {
      return max_tcs_per_module_and_column_.at(moduleId).at(theColumnIndex).second;
    }  //Get TC budget for column at index theColumnIndex in the vector
    int getColFromBudgetMapAtIndex(unsigned moduleId, unsigned theColumnIndex) const {
      return max_tcs_per_module_and_column_.at(moduleId).at(theColumnIndex).first;
    }  //Get column number at index theColumnIndex in the vector
    unsigned getChannelAtIndex(unsigned moduleId, int theColumn, unsigned theChnFrameIndex) const {
      return chn_frame_slots_per_mod_and_col_.at(moduleId).at(theColumn).at(theChnFrameIndex).first;
    }  //Extract channel number for colnr theColumn, at given channel+frame index in the vector
    unsigned getFrameAtIndex(unsigned moduleId, int theColumn, unsigned theChnFrameIndex) const {
      return chn_frame_slots_per_mod_and_col_.at(moduleId).at(theColumn).at(theChnFrameIndex).second;
    }  //Extract frame number for colnr theColumn, at given channel+frame index in the vector
    std::vector<std::pair<unsigned, unsigned>> getTDAQSlotsForModule(unsigned moduleId) const { return tdaq_slots_per_bin_per_mod_.at(moduleId); }
    unsigned isModuleReadByTDAQ(unsigned moduleId) const { return tdaq_slots_per_bin_per_mod_.count(moduleId); }

  private:
    unsigned sector120_;
    uint32_t fpga_id_;
    std::unordered_map<unsigned, std::vector<std::pair<int, unsigned>>> max_tcs_per_module_and_column_;
    std::unordered_map<unsigned, std::unordered_map<int, std::vector<std::pair<unsigned, unsigned>>>> chn_frame_slots_per_mod_and_col_;
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>> tdaq_slots_per_bin_per_mod_;
    //const uint32_t dummyModId_ = 1879048191;  // Just to avoid filling maps for random module ID values. Temporary!!
  };

}  // namespace l1thgcfirmware

#endif
