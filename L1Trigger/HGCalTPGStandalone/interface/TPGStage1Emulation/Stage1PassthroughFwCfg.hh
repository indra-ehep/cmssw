#ifndef Stage1PassthroughFwCfg_h
#define Stage1PassthroughFwCfg_h

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cassert>

namespace TPGStage1Emulation {

class Stage1PassthroughFwCfg : public Stage1IOFwCfg {
public:
  enum {
    NumberOfEconts=34*32*32
  };
  
  Stage1PassthroughFwCfg() {
    for(unsigned lp(0);lp<MaximumLpgbtPairs;lp++) {
      _outputLink[lp]=8*lp/MaximumLpgbtPairs;
      if(_outputLink[lp]>3) _outputLink[lp]=7-_outputLink[lp];
      
      for(unsigned up(0);up<MaximumUnpackersPerLpgbtPair;up++) {
	_outputNumberOfTCs[lp][up]=3;
      }
    }
  }

  unsigned outputLink(unsigned lp) const {
    assert(lp<MaximumLpgbtPairs);
    
    return _outputLink[lp];
  }
  
  unsigned outputNumberOfTCs(unsigned lp, unsigned up) const {
    assert(lp<MaximumLpgbtPairs);
    assert(up<MaximumUnpackersPerLpgbtPair);
    
    return _outputNumberOfTCs[lp][up];
  }
  
  void print() const {
    std::cout << "Stage1PassthroughFwCfg(" << this << ")::print() ";
    Stage1IOFwCfg::print();
    
    std::cout << std::endl;
    for(unsigned lp(0);lp<MaximumLpgbtPairs;lp++) {
      std::cout << " LpGBT pair " << std::setw(2) << lp
		<< ", output link " << _outputLink[lp];
      std::cout << ", number of TCs";
      for(unsigned up(0);up<MaximumUnpackersPerLpgbtPair;up++) {
	std::cout << " " << std::setw(2) << _outputNumberOfTCs[lp][up];
      }
      std::cout << std::endl;
    }
  }    

private:
  unsigned _outputLink[MaximumLpgbtPairs];
  unsigned _outputNumberOfTCs[MaximumLpgbtPairs][MaximumUnpackersPerLpgbtPair];
};

}
#endif
