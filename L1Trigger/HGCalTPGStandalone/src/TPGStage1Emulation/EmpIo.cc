
#ifdef USE_EMP_IO
#include "emp/BoardDataIO.hpp"
#endif

//#include "FE.hh"
//#include "Stage1IO.hh"
//#include "Stage1Passthrough.hh"
#include "../../interface/TPGBEDataformat.hh"
#include "../../interface/TPGFEDataformat.hh"
#include "../../interface/TPGStage1Emulation/EmpIo.hh"


void EmpIo::readEmpFileStage1OutputRaw(const std::string &fn,
				std::vector< std::vector<Stage1OutputPacketRaw> > &vS1) {

#ifdef USE_EMP_IO  
  bool doPrint(true);
  
  emp::BoardDataReader bdr(fn);
  emp::BoardData bd(bdr.get(0));
  std::vector<uint32_t> vLinks(bd.links());

  if(doPrint) std::cout << "Nlinks = " << vLinks.size() << std::endl;

  unsigned nWord;
  unsigned nPacket;
  
  uint16_t *d;
  
  for(unsigned lk(0);lk<vLinks.size();lk++) {
    if(vLinks[lk]>=26 && vLinks[lk]<=29) { 
      if(doPrint) std::cout << "Link = " << lk
			    << ", number = " << vLinks[lk]
			    << std::endl;

      bool synched(false);
      bool invalid(false);

      unsigned nLk(vLinks[lk]);
      nPacket=0;
      
      vS1.push_back(std::vector<Stage1OutputPacketRaw>());
      vS1.push_back(std::vector<Stage1OutputPacketRaw>());
      
      const emp::LinkData &link(bd.link(vLinks[lk]));
      emp::LinkData::const_iterator iter(link.begin());
      
      while(iter!=link.end()) {
	const emp::Frame &f(*iter);
	
	if(doPrint) std::cout << "Link = " << lk << ", word " << f
			      << std::dec << std::endl;
	
	if(f.valid) {
	  if(invalid || f.startOfPacket) {
	    if(doPrint && synched) {
	      vS1[vS1.size()-2][nPacket-1].print();
	      //vS1[vS1.size()-2][nPacket-1][1].print();
	      vS1[vS1.size()-1][nPacket-1].print();
	      //vS1[vS1.size()-1][nPacket-1][1].print();
	    }
	    
	    nWord=0;
	    synched=true;
	    vS1[vS1.size()-2].push_back(Stage1OutputPacketRaw());
	    vS1[vS1.size()-1].push_back(Stage1OutputPacketRaw());
	    nPacket++;
	  }
	  
	  if(synched) {
	    assert(nWord<8);
	    if(nWord==0) {
	      d=vS1[vS1.size()-2][nPacket-1].setMsData(0);
	      d[nWord]=(f.data    )&0xffff;
	      d=vS1[vS1.size()-2][nPacket-1].setMsData(1);
	      d[nWord]=(f.data>>16)&0xffff;
	      d=vS1[vS1.size()-1][nPacket-1].setMsData(0);
	      d[nWord]=(f.data>>32)&0xffff;
	      d=vS1[vS1.size()-1][nPacket-1].setMsData(1);
	      d[nWord]=(f.data>>48)&0xffff;

	    } else {
	      d=vS1[vS1.size()-2][nPacket-1].setTcData(0);
	      d[nWord-1]=(f.data    )&0xffff;
	      d=vS1[vS1.size()-2][nPacket-1].setTcData(1);
	      d[nWord-1]=(f.data>>16)&0xffff;
	      d=vS1[vS1.size()-1][nPacket-1].setTcData(0);
	      d[nWord-1]=(f.data>>32)&0xffff;
	      d=vS1[vS1.size()-1][nPacket-1].setTcData(1);
	      d[nWord-1]=(f.data>>48)&0xffff;
	    /*
	    vS1[vS1.size()-][nPacket-1][lk-26][0].setData(nWord,(f.data    )&0xffff);
	    vS1[vS1.size()-][nPacket-1][lk-26][1].setData(nWord,(f.data>>16)&0xffff);
	    vS1[vS1.size()-][nPacket-1][lk-25][0].setData(nWord,(f.data>>32)&0xffff);
	    vS1[vS1.size()-][nPacket-1][lk-25][1].setData(nWord,(f.data>>48)&0xffff);
	    */
	    }
	    nWord++;
	  }
	}
	invalid=!f.valid;
	
	iter++;      
      }
    }
  }


#endif
}

void EmpIo::readEmpFileStage1Output(const std::string &fn,
			     std::vector< std::vector< std::pair< std::array<unsigned,6>,Stage1OutputPacket> > > &vS1) {
  
#ifdef USE_EMP_IO
  bool doPrint(false);
  emp::BoardDataReader bdr(fn);
  emp::BoardData bd(bdr.get(0));
  std::vector<uint32_t> vLinks(bd.links());

  if(doPrint) std::cout << "Nlinks = " << vLinks.size() << std::endl;

  for(unsigned lk(0);lk<vLinks.size();lk++) {
    if(doPrint) std::cout << "Link = " << lk
			  << ", number = " << vLinks[lk]
			  << std::endl;
  }
#endif
}

void EmpIo::readEmpFileElinks(const std::string &fn, std::string &sid,
		       std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > &vEp) {



  vEp.resize(0);

#ifdef USE_EMP_IO
  bool doPrint(false);
  sid="Unknown"; // FIXME
  
  emp::BoardDataReader bdr(fn);
  emp::BoardData bd(bdr.get(0));
  std::vector<uint32_t> vLinks(bd.links());

  if(doPrint) std::cout << "Nlinks = " << vLinks.size() << std::endl;
  
  unsigned nWord;
  
  for(unsigned lk(0);lk<vLinks.size();) {
    if(doPrint) std::cout << "Links = " << lk << ", " << lk+1
			  << ", numbers = " << vLinks[lk] << ", "
			  << vLinks[lk+1] << std::endl;

    assert((vLinks[lk]%2)==0);          // FIXME:should allow single links
    assert(vLinks[lk+1]==vLinks[lk]+1); // FIXME:should allow single links
    
    vEp.push_back(std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> >());
    vEp.back().first=vLinks[lk]/2;
    
    for(unsigned k(0);k<2;k++) {
      unsigned nPacket(0);
      
      bool synched(false);

      const emp::LinkData &link(bd.link(vLinks[lk+k]));
      emp::LinkData::const_iterator iter(link.begin());
      
      while(iter!=link.end()) {
	const emp::Frame &f(*iter);
      
	if(doPrint) std::cout << "Link = " << lk+k << ", word " << f
			      << std::dec << std::endl;
	
	if(f.valid) {
	  if(f.startOfPacket) {
	    
	    if(synched && nWord<7) {
	      for(;nWord<7;nWord++) {
		vEp.back().second[nPacket-1][nWord+7*k]=0xffffffff;
	      }
	    }
	    
	    nWord=0;
	    synched=true;
	    if(k==0) vEp.back().second.push_back(TPGFEDataformat::OrderedElinkPacket());
	    nPacket++;
	  }

	  if(synched) {
	    assert(nWord<7*(k+1));
	    vEp.back().second[nPacket-1][nWord+7*k]=f.data;
	    nWord++;
	  }
	}
	iter++;      
      }
    }
    lk+=2;
  }

#else

  std::ifstream fin(fn.c_str());
  std::string temp;
  fin >> temp >> sid;
  std::cout << "Temp = " << temp << ", sid = " << sid << std::endl;

  for(unsigned l(0);l<13;l++) {
    fin >> temp;
  std::cout << "Temp = " << temp << std::endl;
  }
  assert(temp=="Link");
  
  unsigned nl;
  for(unsigned l(0);l<128;l++) {
    fin >> temp;
    std::cout << "N Temp = " << temp << std::endl;
    if(temp=="Frame") break;

    std::istringstream sin(temp);
    sin >> nl;
    std::cout << "N Temp = " << temp << ", nl = " << nl << std::endl;
    if((nl%2)==0) {
      vEp.push_back(std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> >());
      vEp.back().first=nl/2;
    } else {
      assert(nl==2*vEp.back().first+1);
    }
  }

  


  
#endif
}

void EmpIo::writeEmpFileElinks(const std::string &fn, const std::string &sid,
			const std::vector< std::pair<unsigned,std::vector<TPGFEDataformat::OrderedElinkPacket> > > &vEp) {

  //bool doPrint(false);

#ifdef USE_EMP_IO
  
  emp::BoardDataWriter bdw(fn);
  emp::BoardData bd(sid);

  emp::Frame f;
  f.strobe=true;
  
  for(unsigned i(0);i<vEp.size();i++) {
    emp::LinkData ld[2];

    for(unsigned j(0);j<vEp[i].second.size();j++) {
      for(unsigned k(0);k<14;k++) {
	f.startOfPacket=(k==0 || k==7);
	f.endOfPacket=(k==6 || k==13);
	f.startOfOrbit=((k==0 || k==7) && (vEp[i].second[j][0]>>28)==0xf);
	f.valid=true;
	f.data=vEp[i].second[j][k];
	ld[k/7].push_back(f);
      }
      
      f.startOfPacket=false;
      f.endOfPacket=false;
      f.startOfOrbit=false;
      f.valid=false;
      f.data=0xffffffffffffffff;
      ld[0].push_back(f);
      ld[1].push_back(f);
    }

    bd.add(2*vEp[i].first ,ld[0]);
    bd.add(2*vEp[i].first+1,ld[1]);
  }
  
  bdw.put(bd);

#else

  unsigned strobe(0);
  std::ofstream fout(fn.c_str());
  /*
  unsigned linkNumber[116];

  //0-19, 24-27, 36-127
  for(unsigned i( 0);i< 20;i++) linkNumber[i]=i;
  for(unsigned i(20);i< 24;i++) linkNumber[i]=i+4;
  for(unsigned i(24);i<116;i++) linkNumber[i]=i+12;

  fout.open
  */
  fout << "ID: " << sid << std::endl
       << "Metadata: (strobe,) start of orbit, start of packet, end of packet, valid" << std::endl << std::endl
       << "      Link" << std::setfill('0');

  unsigned nBx(0);
  
  for(unsigned i(0);i<vEp.size();i++) {
    if(i==0) nBx=vEp[i].second.size();
    assert(nBx==vEp[i].second.size());
    
    std::ostringstream sout;
    sout << std::setfill('0')
	 << "              " << (strobe==0?"":" ")
	 << std::setw(3) << 2*vEp[i].first   << "      "
	 << "              " << (strobe==0?"":" ")
	 << std::setw(3) << 2*vEp[i].first+1 << "      ";
    fout << sout.str();
  }
  
  fout << std::endl;
  
  for(unsigned j(0);j<nBx;j++) {
    for(unsigned k(0);k<8;k++) {
      fout << "Frame " << std::setw(4) << 8*j+k << "  ";
    
      for(unsigned i(0);i<vEp.size();i++) {
	for(unsigned l(0);l<2;l++) {
	  uint16_t meta(0);
	  uint64_t data(0xffffffffffffffff);

	  if(k<7) {
	    meta=1;
	    if(k==0) meta+=100;
	    if(k==0 && (vEp[i].second[j][0]>>28)==0xf) meta+=1000;
	    if(k==6) meta+=10;
	    data=vEp[i].second[j][7*l+k];
	  }

	  fout << "  " << std::setw(4) << meta << " "
	       << std::hex
	       << std::setw(16) << data
	       << std::dec;
	}
      }
      fout << std::endl;
    }
  }
  
  fout.close();

#endif
}
