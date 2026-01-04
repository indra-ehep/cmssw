/*
  g++ emul/GenerateEmpRxFile.cpp -o GenerateEmpRxFile.exe `root-config --libs --cflags`

  
  export LD_LIBRARY_PATH=/opt/cactus/lib:/opt/smash/lib

  g++ -I /opt/cactus/include CaptureAnalysis/GenerateEmpRxFile.cpp -DUSE_EMP_IO -o GenerateEmpRxFile.exe -L /opt/cactus/lib  -l cactus_emp -l cactus_uhal_grammars -l cactus_emp_logger -l serenity_core -l cactus_uhal_uhal -l cactus_uhal_tests -l cactus_uhal_log


g++ -I TPGStage1Emulation -I. GenerateEmpRxFile.cpp -L /opt/local/lib -l yaml-cpp `root-config --libs --cflags` -I`root-config --incdir` -o GenerateEmpRxFile.exe
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <random>

//#include "FE.hh"
#include "../../interface/TPGFEEmulation/TPGFEModuleEmulation.hh"
#include "../../interface/TPGStage1Emulation/Stage1Passthrough.hh"
#include "../../interface/TPGStage1Emulation/Stage1PassthroughFwCfg.hh"
#include "../../interface/TPGStage1Emulation/EmpIo.hh"

typedef TPGStage1Emulation::Stage1Passthrough Stage1;
/*
void FillRandomTcRawData(TPGFEDataformat::TcRawData::Type type,
			 unsigned nTc,
			 TPGFEDataformat::TcRawDataPacket &vtcrp) {

  std::vector<TPGFEDataformat::TcRawData> &vtc(vtcrp.second);
  
  if(type==TPGFEDataformat::TcRawData::BestC) {
    vtc.resize(nTc+1);
    vtc[0].setModuleSum(rand()&0xff);
    
    unsigned step(48/nTc);
    for(unsigned i(1);i<vtc.size();i++) {
      vtc[i].setTriggerCell(type,step*(i-1)+(rand()%step),rand()&0x7f);
    }
    
  } else if(type==TPGFEDataformat::TcRawData::STC4A) {
    vtc.resize(nTc);
    for(unsigned i(0);i<vtc.size();i++) {
      vtc[i].setTriggerCell(type,rand()&0x3,rand()&0x7f);
    }
    
  } else if(type==TPGFEDataformat::TcRawData::STC4B) {
    vtc.resize(nTc);
    for(unsigned i(0);i<vtc.size();i++) {
      vtc[i].setTriggerCell(type,rand()&0x3,rand()&0x1ff);
    }
    
  } else if(type==TPGFEDataformat::TcRawData::STC16) {
    vtc.resize(nTc);
    for(unsigned i(0);i<vtc.size();i++) {
      vtc[i].setTriggerCell(type,rand()&0xf,rand()&0x1ff);
    }
    
  } else {
    assert(false);
  }
}
*/
int main() {
  TPGStage1Emulation::Stage1PassthroughFwCfg fwCfg;

  std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > empRx(58);

  unsigned linkN[128];

  unsigned ln(0);
  for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {
    bool connected(false);
    for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
      if(fwCfg.connected(lp,up)) connected=true;
    }

    if(connected) {
      assert(ln<58);
      std::cout << "ln = " << ln << std::endl;
      linkN[lp]=ln;
      empRx[ln].first=lp;
      ln++;
    }
  }
  assert(ln==58);
  
  TPGFEDataformat::TcRawDataPacket vTc;
  TPGFEDataformat::OrderedElinkPacket vEl;

  unsigned bxi;
  unsigned obx(8);
  for(unsigned ibx(0);ibx<128;ibx++) {
    unsigned bx=obx+ibx;

    if(bx==0 || bx>=3564) bxi=0xf;
    else bxi=(bx%8);
    //else bxi=7-(bx%8); // Unsynch

    std::cout << "BX loop = " << bx << ", internal " << bxi << std::endl;

    for(unsigned lp(0);lp<TPGStage1Emulation::Stage1IOFwCfg::MaximumLpgbtPairs;lp++) {
      bool connected(false);

      for(unsigned up(0);up<TPGStage1Emulation::Stage1IOFwCfg::MaximumUnpackersPerLpgbtPair;up++) {
        if(fwCfg.connected(lp,up)) {
	  connected=true;
	  //vTc.resize(0);

	  TPGFEDataformat::Type type=fwCfg.type(lp,up);
	  unsigned nTc=fwCfg.numberOfTCs(lp,up);

	  //if(lp==0) std::srand(0x12345678);
	  TPGFEModuleEmulation::ECONTEmulation::generateRandomTcRawData(bx,type,nTc,vTc);
	  //TPGFEModuleEmulation::ECONTEmulation::generateZeroEnergyTcRawData(bx,type,nTc,vTc);

	  std::cout << std::endl << "lp,up = " << lp << ", " << up
		    << ", TCs generated" << std::endl;
    vTc.print();
	
	  TPGFEModuleEmulation::ECONTEmulation::convertToElinkData(bxi,vTc,vEl.data()+fwCfg.firstElink(lp,up));
	
	  if(ibx==0 && lp==0) {
	    std::cout << std::endl << "ECONT " << up << std::endl;
	    vTc.print();
      std::cout << std::hex << std::setfill('0');
	    for(unsigned i(0);i<vEl.size();i++) {
	      std::cout << " 0x" << vEl[i];
	    }
	    std::cout << std::dec << std::setfill(' ') << std::endl;
	  }

	}
      }
      
      if(connected) {
	empRx[linkN[lp]].second.push_back(vEl);
      
	std::cout << std::endl << "ECONT " << lp << std::endl;
	std::cout << std::hex << std::setfill('0');
	for(unsigned i(0);i<vEl.size();i++) {
	  std::cout << " 0x" << vEl[i];
	}
	std::cout << std::dec << std::setfill(' ') << std::endl;
      }
    }

  }

  std::cout << "Writing file" << std::endl;
  EmpIo::writeEmpFileElinks("rx_test2.txt","Emulator",empRx);

  std::string sid;
  std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > rb;
  std::cout << "Reading back file" << std::endl;
  EmpIo::readEmpFileElinks("rx_test2.txt",sid,rb);

  return 0;
}
