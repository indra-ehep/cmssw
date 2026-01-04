
#ifndef EmpIo_h
#define EmpIo_h

#include <iostream>
#include <iomanip>
#include <fstream>

typedef std::array<TPGBEDataformat::Stage1ToStage2Data,6> Stage1OutputPacket;
typedef TPGBEDataformat::UnpackerOutputStreamPair Stage1OutputPacketRaw;

class EmpIo {
public:
  static void readEmpFileStage1OutputRaw(const std::string &fn,
				  std::vector< std::vector<Stage1OutputPacketRaw> > &vS1);
  static void readEmpFileStage1Output(const std::string &fn,
			       std::vector< std::vector< std::pair< std::array<unsigned,6>,Stage1OutputPacket> > > &vS1);
  static void readEmpFileElinks(const std::string &fn, std::string &sid,
			 std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > &vEp);


  static void writeEmpFileElinks(const std::string &fn, const std::string &sid,
			  const std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > &vEp);
private:

};

#endif
