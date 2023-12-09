#ifndef hgcal_econt_Configuration_h
#define hgcal_econt_Configuration_h

#include <iostream>
#include <iomanip>
#include <fstream>
//#include <sstream>
#include <cstdint>
#include <cassert>

namespace hgcal_econt{
  
  class ConfigurationChannel {
  public:
    ConfigurationChannel() {}
  
    bool ldm() const {
      return ldm_;
    }    
    void ldm(bool isldm) {
      ldm_ = isldm ;
    }

    unsigned dropLsb() const {
      return dropLsb_;
    }
    void dropLsb(unsigned lsbbit) {
      dropLsb_ = lsbbit;
    }
  
    uint16_t calibration() const {
      return calibration_;
    }
    void calibration(uint16_t  calib) {
      calibration_ = calib;
    }

    uint32_t threshold() const {
      return threshold_;
    }
    void threshold(uint32_t thresh) {
      threshold_ = thresh;
    }

    uint8_t quadSetting() const{
      return quadSetting_ ;
    }
    void quadSetting(uint8_t quad) {
      quadSetting_ = quad;
    }
    
    uint8_t userWord(unsigned n) const {
      assert(n<4);
      return userWord_[n];
    }
  
    void print() const {
      std::cout << "ConfigurationChannel::print()  Channel "
		<< channel_ << std::endl;
      std::cout << " Module = " << (ldm_?"LDM":"HDM")
		<< ", drop LSB = " << dropLsb_ << std::endl;    
      std::cout << " Quad setting = 0x"
		<< std::hex << std::setfill('0')
		<< std::setw(2) << uint16_t(quadSetting_)
		<< ", user words " << (userWordsEnable()?"enabled":"disabled")
		<< ", PRBS " << (prbsEnable()?"enabled":"disabled") << std::endl;
      std::cout << " PRBS seed = 0x"
		<< std::setw(4) << prbsSeed_ << std::endl;
      std::cout << " Calibration = 0x"
		<< std::setw(4) << calibration_ << "(~= "
		<< calibration_/2048.0 << ")"
		<< std::endl;
      std::cout << " Threshold = 0x"
		<< std::setw(6) << threshold_ << std::endl;
      std::cout << " User words =";
      for(unsigned n(0);n<4;n++) {
	std::cout << " 0x" << std::setw(2) << uint16_t(userWord_[n]);
      }    
      std::cout << std::dec << std::setfill(' ') << std::endl;
    }

    bool userWordsEnable() const {
      return (quadSetting_&0x14)==0x14;
    }
  
    bool prbsEnable() const {
      return (quadSetting_&0x0c)==0x0c;
    }

    // private:
    unsigned channel_;
    bool ldm_;
    unsigned dropLsb_;
    uint8_t quadSetting_;
    uint8_t userWord_[4];
    uint16_t prbsSeed_;
    uint16_t calibration_;
    uint32_t threshold_;  
  };

  class Configuration {
  public:
    enum {
      NumberOfBytes=0x630
    };
  
    Configuration() {}
  
    bool readFile(const std::string &f, bool serenity=false) {
      std::cout << "Reading ";
      if(serenity) std::cout << "Serenity ";
      std::cout << "file " << f << std::endl;
      std::ifstream fin(f.c_str());
      assert(fin);

      fin >> std::hex;

      if(!serenity) {
	unsigned v;
	for(unsigned b(0);b<NumberOfBytes;b++) {      
	  fin >> v;
	  byte_[b]=v&0xff;
	}

      } else {
	char c;
	uint16_t a;
	unsigned v;
	for(unsigned b(0);b<NumberOfBytes;b++) {      
	  fin >> c >> c >> a >> c >> c >> c >> v;
	  /*
	    std::cout << "-" << c[0] << "-"
	    << " -" << c[1] << "-"
	    << " -" << c[2] << "-"
	    << " -" << c[3] << "-"
	    << " -" << c[4] << "- "
	    << a << " " << unsigned(v) << std::endl;
	  */
	  byte_[b]=v&0xff;
	}
      }

      return true;
    }

    bool writeFile(const std::string &f) {
      std::cout << "Writing " << f << std::endl;
      std::ofstream fout(f.c_str());
      assert(fout);

      fout << std::hex << std::setfill('0');

      unsigned v;
      for(unsigned b(0);b<NumberOfBytes;b++) {      
	v=byte_[b];
      
	if((b%16)>0) fout << " ";
	fout << std::setw(2) << v;
	if((b%16)==15) fout << std::endl;
      }

      return true;
    }

    uint8_t byte(unsigned b) const {
      assert(b<NumberOfBytes);
      return byte_[b];
    }

    void setByteTo(unsigned b, uint8_t v) {
      assert(b<NumberOfBytes);
      byte_[b]=v;
    }
  
    uint16_t bytes2(unsigned b) const {
      assert(b<NumberOfBytes-1);
      uint16_t v(byte_[b+1]);
      return (v<<8)|byte_[b];
    }

    void setBytes2To(unsigned b, uint16_t v) {
      assert(b<NumberOfBytes-1);
      byte_[b]=v&0xff;
      byte_[b+1]=v>>8;
    }
  
    uint32_t bytes3(unsigned b) const {
      assert(b<NumberOfBytes-2);
      uint32_t v(byte_[b+2]);
      v=(v<<8)|byte_[b+1];
      return (v<<8)|byte_[b];
    }
  
    void setBytes3To(unsigned b, uint32_t v) {
      assert(b<NumberOfBytes-1);
      byte_[b]=v&0xff;
      byte_[b+1]=(v>>8)&0xff;
      byte_[b+2]=(v>>16)&0xff;
    }
  
    uint32_t bytes4(unsigned b) const {
      assert(b<NumberOfBytes-3);
      uint32_t v(byte_[b+3]);
      v=(v<<8)|byte_[b+2];
      v=(v<<8)|byte_[b+1];
      return (v<<8)|byte_[b];
    }
  
    uint64_t bytes5(unsigned b) const {
      assert(b<NumberOfBytes-4);
      uint64_t v(byte_[b+4]);
      v=(v<<8)|byte_[b+3];
      v=(v<<8)|byte_[b+2];
      v=(v<<8)|byte_[b+1];
      return (v<<8)|byte_[b];
    }
  
    uint8_t quadSetting(unsigned qd) const {
      assert(qd<12);
      return byte_[0x040*qd];
    }

    void setQuadSettingTo(unsigned qd, uint8_t s) {
      assert(qd<12);
      byte_[0x040*qd]=s;    
    }
  
    uint16_t prbsSeed(unsigned qd) const {
      assert(qd<12);
      return bytes2(0x040*qd+0x002);
    }
  
    uint8_t userWord(unsigned ch, unsigned n) const {
      assert(ch<48);
      uint32_t w(bytes4(0x040*(ch/4)+0x004+0x004*n));
      return (w>>(7*(ch%4)))&0x7f;
    }

    uint16_t calibration(unsigned ch) const {
      assert(ch<48);
      return bytes2(0x3f4+2*ch);
    }
  
    bool ldm() const {
      return (byte_[0x454]&0x8)==0;
    }

    unsigned algorithm() const {
      return byte_[0x454]&0x7;
    }

    uint32_t threshold(unsigned ch) const {
      assert(ch<48);
      return bytes3(0x455+3*ch);
    }  
 
    void setThresholdTo(unsigned ch, uint32_t t) {
      assert(ch<48);
      setBytes3To(0x455+3*ch,t);
    }  
 
    uint16_t idlePattern() const {
      return bytes2(0x3a9)&0x7ff;
    }  
 
    uint16_t bufferT1() const {
      return bytes2(0x3ab);
    }  

    uint16_t t1Latency() const {
      return bufferT1()/(2*numberOfElinks());
    }  

    void setBufferT1To(uint16_t t) {
      setBytes2To(0x3ab,t);
    }
  
    uint16_t bufferT2() const {
      return bytes2(0x3ad);
    }  
 
    uint8_t bufferT3() const {
      return byte_[0x3af];
    }  
 
    unsigned stcType() const {
      return byte_[0x3b0]&0x3;
    }

    bool moduleSumAllTcs() const {
      return (byte_[0x3b0]&0x4)==0x4;
    }

    bool runEnable() const {
      return (byte_[0x4f5]&0x2)==0x2;
    }

    unsigned numberOfElinks() const {
      return byte_[0x3b0]>>4;
    }

    void setNumberOfElinksTo(unsigned n) {
      assert(n>0 && n<=12);
      byte_[0x3b0]=(byte_[0x3b0]&0xf)|(n<<4);
    }
  
    unsigned dropLsb() const {
      unsigned d(byte_[0x4e5]&0x7);
      if(d<=4) return d;
      return 0;
    }

    unsigned bcrResetValue() const {
      return bytes2(0x394);
    }
  
    unsigned bxPeriod() const {
      return bytes2(0x392);
    }
  
    ConfigurationChannel configurationChannel(unsigned ch) const {
      assert(ch<48);
      ConfigurationChannel cc;
      cc.channel_=ch;
      cc.ldm_=ldm();
      cc.dropLsb_=dropLsb();
      cc.quadSetting_=quadSetting(ch/4);
      cc.prbsSeed_=prbsSeed(ch/4);
      for(unsigned n(0);n<4;n++) cc.userWord_[n]=userWord(ch,n);
      cc.calibration_=calibration(ch);
      cc.threshold_=threshold(ch);      
      return cc;
    }
  
    void print() {
      std::cout << "Configuration::print()" << std::endl;
      std::cout << " Module = " << (ldm()?"LDM":"HDM")
		<< ", drop LSB = " << dropLsb()
		<< ", number of elinks = " << numberOfElinks()
		<< std::endl;

      std::cout << " BX reset value = " << bcrResetValue()
		<< ", period = " << bxPeriod()
		<< std::endl;

      std::cout << " Run mode " << (runEnable()?"enabled":"disabled")
		<< std::endl;

      std::cout << " Algorithm = " << algorithm();
      if(algorithm()==0) std::cout << " = threshold";
      if(algorithm()==1) std::cout << " = super TC";
      if(algorithm()==2) std::cout << " = best choice";
      if(algorithm()==3) std::cout << " = repeater";
      if(algorithm()==4) std::cout << " = autoencoder";
      if(algorithm()>=5) std::cout << " = unknown";
      std::cout << ", super TC type = " << stcType();
      if(stcType()==0) std::cout << " = STC4A";
      if(stcType()==1) std::cout << " = STC16";
      if(stcType()==2) std::cout << " = CTC4";
      if(stcType()==3) std::cout << " = STC4B";
      std::cout << std::endl;

      std::cout << " Module sum = "
		<< (moduleSumAllTcs()?"All TCs":"Unselected TCs")
		<< std::endl;
      std::cout << " Buffer thresholds T1 = " << bufferT1()
		<< ", T2 = " << bufferT2()
		<< ", T3 = " << uint16_t(bufferT3())
		<< ", T1 latency = " << t1Latency()
		<< std::endl;
    
      std::cout << " Quad settings:"
		<< std::hex << std::setfill('0') << std::endl;
      std::cout << " ";
      for(unsigned qd(0);qd<12;qd++) {
	std::cout << " 0x" << std::setw(2) << uint16_t(quadSetting(qd));
      }
      std::cout << std::endl;

      std::cout << " PRBS seeds:"
		<< std::hex << std::setfill('0') << std::endl;
      std::cout << " ";
      for(unsigned qd(0);qd<12;qd++) {
	std::cout << " 0x" << std::setw(4) << prbsSeed(qd);
      }
      std::cout << std::endl;

      std::cout << " Calibrations:"
		<< std::hex << std::setfill('0');
      for(unsigned ch(0);ch<48;ch++) {
	if((ch%12)==0) std::cout << std::endl << " ";
	std::cout << " 0x" << std::setw(4) << calibration(ch);
      }
      std::cout << std::endl;
    
      std::cout << " Thresholds:"
		<< std::hex << std::setfill('0');
      for(unsigned ch(0);ch<48;ch++) {
	if((ch%12)==0) std::cout << std::endl << " ";
	std::cout << " 0x" << std::setw(6) << threshold(ch);
      }
      std::cout << std::endl;
    
      std::cout << " User words:"
		<< std::hex << std::setfill('0');
      for(unsigned ch(0);ch<48;ch++) {
	std::cout << std::endl << " ";
	for(unsigned n(0);n<4;n++) {
	  std::cout << " 0x" << std::setw(2) << uint16_t(userWord(ch,n));
	}
      }
      std::cout << std::dec << std::setfill(' ') << std::endl;

      std::cout << " Input MUX words:";
      for(unsigned ch(0);ch<48;ch++) {
	if((ch%12)==0) std::cout << std::endl << " ";
	std::cout << " " << std::setw(2) << unsigned(byte(0x3c4+ch));
      }
      std::cout << std::endl;
      std::cout << std::endl;

      std::cout << " Channels:";
      for(unsigned ch(0);ch<48;ch++) {
	ConfigurationChannel cc(configurationChannel(ch));
	cc.print();
      }
    }
  
  private:
    uint8_t byte_[NumberOfBytes];
  };

  class Status {
  public:
    Status(const Configuration &c) : cfg(c) {}

    uint8_t quadStatus(unsigned qd) const {
      assert(qd<12);
      return cfg.byte(0x040*qd+0x014);
    }
    
    uint8_t quadSelect(unsigned qd) const {
      assert(qd<12);
      return cfg.byte(0x040*qd+0x015);
    }
    
    uint64_t pllStatus() const {
      return cfg.bytes5(0x533);
    }
    
    void print() const {
      std::cout << "Status::print()" << std::endl;
      std::cout << std::hex << std::setfill('0');

      std::cout << " PLL status = 0x" << std::setw(10) << pllStatus()
		<< std::endl;
      std::cout << " Quad status:" << std::endl;
      std::cout << " ";
      for(unsigned qd(0);qd<12;qd++) {
	std::cout << " 0x" << std::setw(2) << uint16_t(quadStatus(qd));
      }
      std::cout << std::endl;
      std::cout << " Quad select:" << std::endl;
      std::cout << " ";
      for(unsigned qd(0);qd<12;qd++) {
	std::cout << " 0x" << std::setw(2) << uint16_t(quadSelect(qd));
      }
      std::cout << std::endl;
      std::cout << std::dec << std::setfill(' ');
    }
  
  private:
    const Configuration &cfg;
  };
}

#endif
