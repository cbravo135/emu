#include<iostream>
#include<stdio.h>
#include<string>
#include<unistd.h> // for sleep()
#include "TMBTester.h"
#include "TMB.h"
#include "CCB.h"
#include "TMB_constants.h"
#include "JTAG_constants.h"

#ifndef debugV //silent mode
#define PRINT(x)
#define PRINTSTRING(x)
#else //verbose mode
#define PRINT(x) std::cout << #x << ":\t" << x << std::endl;
#define PRINTSTRING(x) std::cout << #x << std::endl;
#endif

TMBTester::TMBTester() {
  tmb_ = 0;
  ccb_ = 0;
  MyOutput_ = & std::cout;
}

TMBTester::~TMBTester(){}
//

/////////////////////////////////////////
// General members:
/////////////////////////////////////////
void TMBTester::readreg4(){
  std::cout << "Basic test of TMB communication" << std::endl;
  int regtoread = 4;
  std::cout << "Getting date from register " << regtoread << std::endl;
  int reg = tmb_->ReadRegister(regtoread);

}

void TMBTester::reset() {
  std::cout << "TMBTester: Hard reset through CCB" << std::endl;
  if ( ccb_ ) {
    ccb_->hardReset();  
  } else {
    std::cout << "No CCB defined" << std::endl;
  }
}

bool TMBTester::runAllTests() {
  std::cout << "TMBTester: Beginning full set of TMB self-tests" << std::endl;

  bool AllOK;

  reset();
  //  readreg4();

  bool bootRegOK = testBootRegister();
  bool VMEfpgaDataRegOK = testVMEfpgaDataRegister();
  bool SlotOK = testFirmwareSlot();
  bool DateOK = testFirmwareDate();
  bool TypeOK = testFirmwareType();
  bool VersionOK = testFirmwareVersion();
  bool RevCodeOK = testFirmwareRevCode();
  bool JTAGchainOK = testJTAGchain();
  bool MezzIdOK = testMezzId();
  bool PROMidOK = testPROMid();
  bool PROMpathOK = testPROMpath();
  bool dsnOK = testDSN();
  bool adcOK = testADC();
  bool is3d3444OK = test3d3444();

  AllOK = (bootRegOK &&
	   VMEfpgaDataRegOK &&
           SlotOK &&
	   DateOK &&
	   TypeOK &&
           VersionOK &&
           RevCodeOK &&
           JTAGchainOK &&
           MezzIdOK &&
           PROMidOK &&
           PROMpathOK &&
           dsnOK &&
           adcOK &&
           is3d3444OK);

  if ( AllOK ) {
    std::cout << "TMBTester: This board PASSED all tests...   OKAY" 
              << std::endl;
  } else {
    std::cout << "TMBTester: This board did not pass all tests...  FAIL" 
              << std::endl;
  }  

  return AllOK;
}


/////////////////////////////////////////
// Tests:
/////////////////////////////////////////
bool TMBTester::testBootRegister() {
  std::cout << "TMBTester: testing Boot Register" << std::endl;

  bool testOK = false;
  int dummy;  
  
  unsigned short int BootData;
  dummy = tmb_->tmb_get_boot_reg(&BootData);
  std::cout << "Initial boot contents = " << std::hex << BootData << std::endl;

  unsigned short int write_data, read_data;
  int err_reg = 0;

  // walk through the 16 bits on the register
  for (int ibit=0; ibit<=15; ibit++) {
    write_data = 0x1 << ibit;
    dummy = tmb_->tmb_set_boot_reg(write_data);

    dummy = tmb_->tmb_get_boot_reg(&read_data);

    // Remove the read-only bits:
    read_data &= 0x3FFF;
    write_data &= 0x3FFF;
    
    if ( !compareValues("bootreg bit",read_data,write_data,true) ) {
      err_reg++;
    }
  }

  // Restore boot contents
  dummy = tmb_->tmb_set_boot_reg(BootData);
  dummy = tmb_->tmb_get_boot_reg(&read_data);   //check for FPGA final state
  std::cout << "Final Boot Contents = " << std::hex << read_data << std::endl;    

  testOK = compareValues("Number of boot register errors",err_reg,0,true);  
  return testOK; 
}


bool TMBTester::testHardReset(){
  std::cout << "TMBTester: testing hard reset TMB via boot register" << std::endl;
  std::cout << "NOTE:  Test not needed, as we hard reset by the CCB" << std::endl; 

  bool testOK = false;
  int dummy;

  unsigned short int BootData;
  dummy = tmb_->tmb_get_boot_reg(&BootData);
  std::cout << "Initial boot contents = " << std::hex << BootData << std::endl;

  unsigned short int write_data, read_data;

  write_data = 0x0200;
  dummy = tmb_->tmb_set_boot_reg(write_data);   //assert hard reset

  dummy = tmb_->tmb_get_boot_reg(&read_data);   //check for FPGA not ready
  read_data &= 0x7FFF;                          // remove tdo

  bool FPGAnotReady = 
    compareValues("Hard reset TMB FPGA not ready",read_data,0x0200,true);

  
  write_data = 0x0000;
  dummy = tmb_->tmb_set_boot_reg(write_data);   //de-assert hard reset

  std::cout << "waiting for TMB to reload..." << std::endl;
  dummy = sleep(5);                             

  dummy = tmb_->tmb_get_boot_reg(&read_data);   //check for FPGA not ready
  read_data &= 0x4000;                          // remove bits beside "FPGA ready"

  bool FPGAReady = compareValues("Hard reset TMB FPGA ready",read_data,0x4000,true);

  // Restore boot contents
  dummy = tmb_->tmb_set_boot_reg(BootData);
  dummy = tmb_->tmb_get_boot_reg(&read_data);   //check for FPGA final state
  std::cout << "Final Boot Contents = " << std::hex << read_data << std::endl;    


  testOK = (FPGAnotReady &&
	    FPGAReady);
  return testOK;
}


bool TMBTester::testVMEfpgaDataRegister(){
  std::cout << "TMBTester: testing VME FPGA Data Register" << std::endl;
  bool testOK = false;

  // Get current status:
  int vme_cfg = tmb_->ReadRegister(vme_phos4a_adr);

  int write_data,read_data;
  bool tempBool;

  int register_error = 0;

  for (int i=0; i<=15; i++) {
    write_data = 1 << i;

    tmb_->WriteRegister(vme_phos4a_adr,write_data);  //write walking 1
    read_data = tmb_->ReadRegister(0);               //read base to purge bit3 buffers
    read_data = tmb_->ReadRegister(vme_phos4a_adr);  //read walking 1
    
    tempBool = compareValues("Register value",read_data,write_data,true);
    
    if (!tempBool) register_error++;
  }

  //restore data register...
  tmb_->WriteRegister(vme_phos4a_adr,vme_cfg);

  write_data = 0x001A;       // turn on cylons
  tmb_->WriteRegister(ccb_cfg_adr,write_data);

  testOK = compareValues("Number of VME FPGA data reg errors",register_error,0,true);
  return testOK;
}


bool TMBTester::testFirmwareSlot(){
  std::cout << "TMBTester: testing TMB slot number" << std::endl;
  int firmwareData = tmb_->FirmwareVersion();
  int slot = (firmwareData>>8) & 0xf;

  std::cout << "TMBTester: TMB Slot (counting from 0) = " 
	    << std::dec << slot << std::endl;

  //VME counts from 0, xml file counts from 1:
  int slotToCompare = (*TMBslot) - 1;

  bool testOK = compareValues("TMB slot",slot,slotToCompare,true);
  return testOK;
}


bool TMBTester::testFirmwareDate() {
  std::cout << "TMBTester: testing Firmware date" << std::endl;

  int firmwareData = tmb_->FirmwareDate();
  int day = firmwareData & 0xff;
  int month = (firmwareData>>8) & 0xff;
  int year=tmb_->FirmwareYear();

  bool testOK = compareValues("Firmware Year",year,0x2005,true);
  return testOK; 
}


bool TMBTester::testFirmwareType() {
  std::cout << "TMBTester: testing Firmware Type" << std::endl;
  bool TypeNormal=false;
  bool TypeDebug=false;

  int firmwareData = tmb_->FirmwareVersion();
  int type = firmwareData & 0xf;

  TypeNormal = compareValues("Firmware Normal",type,0xC,true);
  if (!TypeNormal){
    TypeDebug = compareValues("CAUTION Firmware Debug",type,0xD,true);    
  }
  if (!TypeNormal && !TypeDebug ){
    std::cout << 
      "What kind of Firmware is this? Firmware = " << type << std::endl;
  }
  return TypeNormal;
}


bool TMBTester::testFirmwareVersion() {
  std::cout << "TMBTester: testing Firmware Version" << std::endl;
  int firmwareData = tmb_->FirmwareVersion();
  int version = (firmwareData>>4) & 0xf;

  bool testOK = compareValues("Firmware Version",version,0xE,true);
  return testOK;
}


bool TMBTester::testFirmwareRevCode(){
  std::cout << "TMBTester: testing Firmware Revision Code" << std::endl;

  int firmwareData = tmb_->FirmwareRevCode();

  int RevCodeDay = firmwareData & 0x001f;
  int RevCodeMonth = (firmwareData>>5) & 0x000f;
  int RevCodeYear = (firmwareData>>9) & 0x0007;
  int RevCodeFPGA = (firmwareData>>12) & 0x000F;

  bool testOK = compareValues("Firmware Revcode FPGA",RevCodeFPGA,0x04,true);
  return testOK;
}


bool TMBTester::testJTAGchain(){
  std::cout << "TMBTester: testing User and Boot JTAG chains" << std::endl;

  bool user = testJTAGchain(0);
  bool boot = testJTAGchain(1);

  bool JTAGchainOK = (user && boot);
  return JTAGchainOK;
}

bool TMBTester::testJTAGchain(int type){
  std::cout << "testJTAGchain: WARNING not yet debugged" << std::endl; 

  bool testOK = false;  

  //Put boot register to power-up state:
  tmb_->WriteRegister(TMB_ADR_BOOT,0);

  unsigned int source_adr = UserOrBootJTAG(type);

  short unsigned int write_data, read_data;
  int write_pattern;
  // Select the FPGA JTAG chain
  int FPGAchain = 0x000C;

  int gpio_adr = 0x000028;

  bool tests[5];

  int dummy;

  //loop over transmit bits... 
  for ( int itx=0; itx<=4; itx++) {  //step through tdi,tms,tck,sel0,sel1

    //write the walking 1 to the JTAG chain...
    write_pattern = 0x1 << itx;
    write_data = write_pattern | (FPGAchain << 3) | (0x1 << 7);
    std::cout << "write_data = " << std::hex << write_data << std::endl;
    dummy = tmb_->tmb_vme_reg(source_adr,&write_data);
    tmb_->WriteRegister(source_adr,write_data);

    //copy the tdi to the tdo through gp_io0...
    read_data = tmb_->ReadRegister(gpio_adr);
    write_data = (read_data >> 1) & 0x1;     //get tdi on gp_io1
    tmb_->WriteRegister(gpio_adr,write_data);  //send it back out on gp_io0

    //Read FPGA chain...
    read_data = (tmb_->ReadRegister(gpio_adr) & 0xF);

    if (itx==0 && read_data==0xF) {
      std::cout << "JTAGchain:  Disconnect JTAG cable" << std::endl;
      pause();
    }

    int pat_expect = 0;
    
    switch (itx) {
    case 0:
      pat_expect = 0x3;
      break;
    case 1:
    case 2:
      pat_expect = write_pattern << 1;
    case 3:
    case 4:
      pat_expect = 0xF;
    }

    if (type == 1) {
      tests[itx] = compareValues("Boot JTAG Chain",read_data,pat_expect,true);
    } else {
      tests[itx] = compareValues("User JTAG Chain",read_data,pat_expect,true);
    }
  }

  testOK = (tests[0] &&
	    tests[1] &&
	    tests[2] &&
	    tests[3] &&
	    tests[4] );

  return testOK;
}


bool TMBTester::testMezzId(){
  std::cout << "TMBTester: Checking Mezzanine FPGA and PROMs ID codes" << std::endl;
  std::cout << "testMezzId() WARNING not yet debugged" << std::endl;

  bool testOK = false;

  //Select FPGA Mezzanine FPGA programming JTAG chain from TMB boot register
  int ichain = 0x0004;
  vme_jtag_anystate_to_rti(TMB_ADR_BOOT,ichain); //Take TAP to RTI

  int opcode;

  int reg_len = 32;  

  //Read Virtex2 FPGA (6-bit opcode) and XC18V04 PROM IDcodes (8-bit opcode)
  for (int chip_id=0; chip_id<=4; chip_id++){
    if (chip_id == 0) {
      opcode = 0x09;                  // FPGA IDcode opcode, expect v0A30093
    } else { 
      opcode = 0xFE;                  // PROM IDcode opcode
    }
    // GREG, TRANSLATE THE FOLLOWING:
    //    vme_jtag_write_ir(TMB_ADR_BOOT,ichain,chip_id,opcode);
    //    vme_jtag_write_dr(TMB_ADR_BOOT,ichain,chip_id,tdi,tdo,reg_len);
  }

  //Interpret ID code  GREG, FIGURE OUT HOW TO TO THE INTERPRET ID CODE PART
  //AFTER DOING THE VME_JTAG STUFF...
  


  return testOK;
}


bool TMBTester::testPROMid(){
  std::cout << "TMBTester: Checking User PROM ID codes" << std::endl;

  std::cout << 
   "TMBTester: testPROMid() NOT YET IMPLEMENTED" 
     << std::endl;

  bool test = false;
  return test;
}


bool TMBTester::testPROMpath(){
  std::cout << "TMBTester: Checking User PROM Data Path" << std::endl;

  std::cout << 
   "TMBTester: testPROMpath() NOT YET IMPLEMENTED" 
     << std::endl;

  bool test = false;
  return test;
}


bool TMBTester::testDSN(){
  std::cout << "TMBTester: Checking Digital Serial Numbers for TMB" 
	    << std::endl;
  bool tmbDSN = testDSN(0);

  std::cout << "TMBTester: Checking Digital Serial Numbers for Mezzanine" 
	    << std::endl;
  bool mezzanineDSN = testDSN(1);

  std::cout << "TMBTester: Checking Digital Serial Numbers for RAT" 
	    << std::endl;
  std::cout << "NOT YET IMPLEMENTED" << std::endl;
  //  bool ratDSN = testDSN(2);
  bool ratDSN = false;

  if (tmbDSN) {
    std::cout << "TMBTester: TMB DSN -> PASS" << std::endl;    
  } else {
    std::cout << "TMBTester: TMB DSN -> FAIL" << std::endl;    
  }

  if (mezzanineDSN) {
    std::cout << "TMBTester: Mezzanine DSN -> PASS" << std::endl;    
  } else {
    std::cout << "TMBTester: Mezzanine DSN -> FAIL" << std::endl;    
  }

  if (ratDSN) {
    std::cout << "TMBTester: RAT DSN -> PASS" << std::endl;    
  } else {
    std::cout << "TMBTester: RAT DSN -> FAIL" << std::endl;    
  }

  bool DSNOK = (tmbDSN &&
                mezzanineDSN &&
                ratDSN);
  return DSNOK;
}

bool TMBTester::testDSN(int BoardType){

  std::bitset<64> dsn;

  // get the digital serial number
  dsn = dsnRead(BoardType);

  // compute the CRC
  int crc = dowCRC(dsn);

  //get the 8 most significant bits (from LSB) to compare with CRC
  int dsntocompare = 0;
  int value;
  for (int bit=63; bit>=56; bit--){
    value = dsntocompare << 1;
    dsntocompare = value | dsn[bit];
  }

  bool crcEqualDSN = compareValues("CRC equal dsn[56-63]",crc,dsntocompare,true);
  bool crcZero = compareValues("CRC value ",crc,0,false);

  bool DSNOK = (crcZero &&
		crcEqualDSN);
  return DSNOK;
}


bool TMBTester::testADC(){
  std::cout << "TMBTester: Checking ADC and status" << std::endl;
  std::cout << "testADC() WARNING not yet debugged" << std::endl;

  bool testOK = false;
  int write_data,read_data;
  int dummy;

  // Voltage status bits...
  int voltage_status = tmb_->ReadRegister(vme_adc_adr);
  
  int vstat_5p0v = voltage_status & 0x1;          // 5.0V power supply OK
  int vstat_3p3v = (voltage_status >> 1) & 0x1;   // 3.3V power supply OK
  int vstat_1p8v = (voltage_status >> 1) & 0x1;   // 1.8V power supply OK
  int vstat_1p5v = (voltage_status >> 1) & 0x1;   // 1.5V power supply OK
  int tcrit      = (voltage_status >> 1) & 0x1;   // FPGA and board temperature OK

  bool test5p0 = compareValues("5.0V status bit",vstat_5p0v,0x1,true);
  bool test3p3 = compareValues("3.3V status bit",vstat_3p3v,0x1,true);
  bool test1p8 = compareValues("1.8V status bit",vstat_1p8v,0x1,true);
  bool test1p5 = compareValues("1.5V status bit",vstat_1p5v,0x1,true);
  bool testTcrit = compareValues("FPGA and board temperatures status bit",tcrit,0x1,true);

  bool voltageDisc = (test5p0 &&
		      test3p3 &&
		      test1p8 &&
		      test1p5 &&
		      testTcrit);

  dummy = sleep(2);

  float adc_voltage[13];

  ADCvoltages(adc_voltage);

  float v5p0	=adc_voltage[0];	      
  float	v3p3	=adc_voltage[1];
  float v1p5core=adc_voltage[2];
  float v1p5tt	=adc_voltage[3];
  float v1p0	=adc_voltage[4];
  float a5p0	=adc_voltage[5];	      
  float	a3p3	=adc_voltage[6];
  float	a1p5core=adc_voltage[7];
  float a1p5tt	=adc_voltage[8];
  float	a1p8rat	=adc_voltage[9];	        // if SH921 set 1-2, loop backplane sends 1.500vtt
  //  float v3p3rat	=adc_voltage[9];	// if SH921 set 2-3
  float	v1p8rat =adc_voltage[10];
  float	vref2   =adc_voltage[11];
  float vzero   =adc_voltage[12];
  float	vref    =adc_voltage[13];

  // temperatures not being read out yet....

  float t_local_f_tmb = 0;
  float t_remote_f_tmb = 0;

  bool v5p0OK     = compareValues("+5.0V TMB      ",v5p0    ,5.010,0.025);
  bool v3p3OK     = compareValues("+3.3V TMB      ",v3p3    ,3.218,0.025);
  bool v1p5coreOK = compareValues("+1.5V core     ",v1p5core,1.506,0.025);
  bool v1p5ttOK   = compareValues("+1.5V TT       ",v1p5tt  ,1.489,0.025);
  bool v1p0OK     = compareValues("+1.0V TT       ",v1p0    ,1.005,0.050);
  bool v1p8ratOK  = compareValues("+1.8V RAT core ",v1p8rat ,1.805,0.025);
  bool vref2OK    = compareValues("+vref/2        ",vref2   ,2.048,0.001);
  bool vzeroOK    = compareValues("+vzero         ",vzero   ,0.0  ,0.001);
  bool vrefOK     = compareValues("+vref          ",vref    ,4.095,0.001);

  float atol = 0.16;
  bool a5p0OK     = compareValues("+5.0A TMB      ",a5p0    ,0.245,atol);
  bool a3p3OK     = compareValues("+3.3A TMB      ",a3p3    ,1.260,atol);
  bool a1p5coreOK = compareValues("+1.5A TMB Core ",a1p5core,0.095,atol);
  bool a1p5ttOK   = compareValues("+1.5A TT       ",a1p5tt  ,0.030,atol*1.5);
  bool a1p8ratOK  = compareValues("+1.8A RAT Core ",a1p8rat ,0.030,atol*5.0);

  float ttol = 0.2;
  bool tlocalOK   = compareValues("T TMB pcb      ",t_local_f_tmb ,75.2,ttol);
  bool tremoteOK  = compareValues("T FPGA chip    ",t_remote_f_tmb,78.8,ttol);

  return testOK;
}


bool TMBTester::test3d3444(){
  std::cout << "TMBTester: Verifying 3d3444 operation" << std::endl;

  std::cout << 
   "TMBTester: test3d3444() NOT YET IMPLEMENTED" 
     << std::endl;

  bool test = false;
  return test;
}


/////////////////////////////////////////
// Functions needed to implement tests:
/////////////////////////////////////////
bool TMBTester::compareValues(std::string TypeOfTest, 
                              int testval, 
                              int compareval,
			      bool equal) {

// test if "testval" is equivalent to "compareval"
// return depends on if you wanted them to be "equal"

  std::cout << "compareValues:  " << TypeOfTest << " -> ";
  if (equal) {
    if (testval == compareval) {
      std::cout << "PASS = " << std::hex << compareval << std::endl;
      return true;
    } else {
      std::cout << "FAIL!" << std::endl;
      std::cout << TypeOfTest 
		<< " expected value = " << std::hex << compareval
		<< ", returned value = " << std:: hex << testval
		<< std::endl;
      return false;
    }
  } else {
    if (testval != compareval) {
      std::cout << "PASS -> " << std::hex << testval 
		<< " not equal to " <<std::hex << compareval 
		<< std::endl;
      return true;
    } else {
      std::cout << "FAIL!" << std::endl;
      std::cout << TypeOfTest 
		<< " expected = returned = " << std::hex << testval
		<< std::endl;
      return false;
    }
  }

}

bool TMBTester::compareValues(std::string TypeOfTest, 
                              float testval, 
                              float compareval,
			      float tolerance) {

// test if "testval" is within "tolerance" of "compareval"...

  std::cout << "compareValues tolerance:  " << TypeOfTest << " -> ";

  float err = (testval - compareval)/compareval;

  if (fabs(err)>tolerance) {
      std::cout << "FAIL!" << std::endl;
      std::cout << TypeOfTest 
		<< " expected = " << compareval 
		<< ", returned = " << testval
		<< " outside of tolerance "<< tolerance
		<< std::endl;
      return false;
  } else {
      std::cout << "PASS!" << std::endl;
      std::cout << TypeOfTest 
		<< " value = " << testval
		<< " within "<< tolerance
		<< " of " << compareval
		<< std::endl;
      return true;
  }

}


int TMBTester::dowCRC(std::bitset<64> DSN) {

  // "Calculate CRC x**8 + x**5 + X**4 +1
  //  for 7-byte Dallas Semi i-button data"
  //    header to dow_crc.for, written by Jonathan Kubik

  int ibit;

  int sr[8];
  //initialize CRC shift register
  for (ibit=0; ibit<=7; ibit++) {
    sr[ibit]=0;
  }

  int x8=0;
  int bit;

  //loop over 56 data bits, LSB first:
  for (ibit=0; ibit<=55; ibit++) {
    x8 = DSN[ibit]^sr[7];
    sr[7] = sr[6];
    sr[6] = sr[5];
    sr[5] = sr[4]^x8;
    sr[4] = sr[3]^x8;
    sr[3] = sr[2];
    sr[2] = sr[1];
    sr[1] = sr[0];
    sr[0] = x8;
  }

  //pack shift register into a byte
  int crc=0;
  for (ibit=0; ibit<=7; ibit++) {
    crc |= sr[ibit] << (7-ibit);
  }

  return crc;
}


unsigned int TMBTester::UserOrBootJTAG(int choose){
  // Choose Source for JTAG commands
  int adr = (choose == 1) ? TMB_ADR_BOOT : vme_usr_jtag_adr;
}

void TMBTester::vme_jtag_anystate_to_rti(int address, int chain) {
  //take JTAG tap from any state to TLR then to RTI

  const int mxbits = 6;

  unsigned char tms[mxbits];
  unsigned char tdi[mxbits];
  unsigned char tdo[mxbits];

  unsigned char tms_rti[mxbits] = {1, 1, 1, 1, 1, 0}; //Anystate to TLR then RTI
  unsigned char tdi_rti[mxbits] = {};

  int iframe = 0;                                   //JTAG frame number

  for (int ibit=1; ibit<=mxbits; ibit++) {       // go from any state to RTI
    tms[iframe]=tms_rti[iframe];                   // take tap to RTI
    tdi[iframe]=tdi_rti[iframe];
    iframe++;                                      
  }

  int step_mode = 1;                               // 1=single step, 0=run
  
  int nframes;
  nframes = mxbits;

  vme_jtag_io_byte(address,chain,nframes,tms,tdi,tdo,step_mode);

  return;
}

void TMBTester::vme_jtag_io_byte(int address, 
				 int chain, 
				 int nframes,
				 unsigned char * tms,
				 unsigned char * tdi,
				 unsigned char * tdo,
				 int step_mode) {

  //	Clocks tck for nframes number of cycles.
  //	Writes nframes of data to tms and tdi on the falling edge of tck.
  //	Reads tdo on the rising clock edge.
  //
  //	Caller passes tms and tdi byte arrays with 1 bit per byte.	
  //	Returned data is also 1 bit per byte. Inefficent,but easy.
  //
  //	tms[]	=	byte's lsb to write to parallel port
  //	tdi[]	=	byte's lsb to write to parallel port
  //	tdo[]	=	bit read back from parallel port, stored in lsb
  //	tck		=	toggled by this routine

  // get current boot register:
  int boot_state = tmb_->ReadRegister(TMB_ADR_BOOT);
  boot_state &= 0x7f80;                 //Clear JTAG bits

  //	Set tck,tms,tdi low, select jtag chain, enable VME control of chain

  int tck_bit = 0;                      //TCK low
  int tms_bit = 0;                      //TMS low
  int tdi_bit = 0;                      //TDI low
  int tdo_bit;
  int jtag_en = 1;                      //Boot register sources JTAG

  int sel0 = chain & 0x1;               //JTAG chain select
  int sel1 = (chain>>1) & 0x1;
  int sel2 = (chain>>2) & 0x1;
  int sel3 = (chain>>3) & 0x1;

  int jtag_word;

  jtag_word = boot_state;      
  jtag_word |= tdi_bit;
  jtag_word |= tms_bit << 1;
  jtag_word |= tck_bit << 2;
  jtag_word |= sel0    << 3;
  jtag_word |= sel1    << 4;
  jtag_word |= sel2    << 5;
  jtag_word |= sel3    << 6;
  jtag_word |= jtag_en << 7;

  tmb_->WriteRegister(TMB_ADR_BOOT,jtag_word);  //write boot register

  int jtag_in,jtag_out;

  //Loop over input data frames

  if (nframes>=1) {                       //no frames to send

    //loop over input data frames...
    for (int iframe=0; iframe<=(nframes-1); iframe++) { //arrays count from 0
      tdo[iframe]=0;                      //clear tdo

      tck_bit = 0x0 << 2;                   //take TCK low
      tms_bit = tms[iframe] << 1;           //TMS bit
      tdi_bit = tdi[iframe];                //TDI bit

      jtag_out = jtag_word & 0x7ff8;           //clear old state
      jtag_out |= tck_bit | tms_bit | tdi_bit;

      tmb_->WriteRegister(TMB_ADR_BOOT,jtag_out);  //write boot register

      jtag_in = tmb_->ReadRegister(TMB_ADR_BOOT);  //read boot register
      tdo[iframe] = (jtag_in << 15) & 0x1;         //extract tdo bit, mask lsb
      tdo_bit = tdo[iframe];

      if (!step_mode) {
	step(tck_bit,tms_bit,tdi_bit,tdo_bit);
      }

      tck_bit = 0x1 << 2;  //Take TCK high, leave tms,tdi as they were
      jtag_out |= tck_bit | tms_bit | tdi_bit;
      tmb_->WriteRegister(TMB_ADR_BOOT,jtag_out);  //write boot register

      jtag_in = tmb_->ReadRegister(TMB_ADR_BOOT);  //read boot register
      tdo[iframe] = (jtag_in << 15) & 0x1;         //extract tdo bit, mask lsb

      if (!step_mode) {
	step(tck_bit,tms_bit,tdi_bit,tdo_bit);
      }      
    }
  }
  jtag_out &= 0xfffb;     //Take TCK low, leave others as they were
  tmb_->WriteRegister(TMB_ADR_BOOT,jtag_out);      //write boot register

  return;
}

void TMBTester::step(int tck, int tms, int tdi, int tdo){

  std::cout << "tck = " << std::hex << tck
	    << "tms = " << std::hex << tms
	    << "tdi = " << std::hex << tdi
	    << "tdo = " << std::hex << tdo
	    << "pause.... enter any number, then return...";
  int dummy;
  std::cin >> dummy;

  return;
}


/////////////////////////////////////
// The following should be in TMB: //
/////////////////////////////////////
std::bitset<64> TMBTester::dsnRead(int type) {

  std::bitset<64> dsn;

  int offset;
  offset = type*5;  // 0=TMB, 1=Mezzanine, 2=RAT

  int i;
  int wr_data, rd_data;
  int idata;

  // init pulse >480usec
  wr_data = 0x0005; 
  wr_data <<= offset; //send it to correct component
  rd_data = dsnIO(wr_data);

  // ROM Read command = serial 0x33:
  for (i=0; i<=7; i++) {
    idata = (0x33>>i) & 0x1;
    wr_data = (idata<<1) | 0x1; //send "serial write pulse" with "serial SM start"
    wr_data <<= offset; 
    rd_data = dsnIO(wr_data);
  }

  // Read 64 bits of ROM data = 0x3 64 times
  for (i=0; i<=63; i++) {
    wr_data = 0x0003; 
    wr_data <<= offset;
    rd_data = dsnIO(wr_data);

    // pack data into dsn[]
    dsn[i] = (rd_data >> (4+offset)) & 0x1;
  }

  return dsn;
}

int TMBTester::dsnIO(int writeData){
  //Single I/O cycle for Digital Serial Number...
  //called by dsnRead...

  int adr = vme_dsn_adr;
  int readData;

  // write the desired data word:
  tmb_->WriteRegister(adr,writeData);

  int tmb_busy,mez_busy,rat_busy;
  int busy = 1;
  int nbusy = 1;

  while (busy) {
    readData = tmb_->ReadRegister(adr);
    
    // check busy on all components:
    tmb_busy = (readData>>3) & 0x1;
    mez_busy = (readData>>8) & 0x1;
    rat_busy = (readData>>13) & 0x1;
    busy = tmb_busy | mez_busy | rat_busy;

    if (nbusy%1000 == 0) {
      std::cout << "dsnIO: DSN state machine busy, nbusy = "
                << nbusy << ", readData = " 
		<< std::hex << readData << std::endl;  
    }
    nbusy++;
  }

  // end previous cycle
  tmb_->WriteRegister(adr,0x0000);

  return readData;
}

void TMBTester::ADCvoltages(float * voltage){

  //Read the ADC of the voltage values ->
  //voltage[0] = +5.0V TMB
  //       [1] = +3.3V TMB
  //       [2] = +1.5V core
  //       [3] = +1.5V TT
  //       [4] = +1.0V TT
  //       [5] = +5.0V Current (A) TMB
  //       [6] = +3.3V Current (A) TMB
  //       [7] = +1.5V core Current (A) TMB
  //       [8] = +1.5V TT Current (A) TMB
  //       [9] = if SH921 set 1-2, +1.8V RAT current (A)
  //           = if SH921 set 2-3, +3.3V RAT
  //      [10] = +1.8V RAT core
  //      [11] = reference Voltage * 0.5
  //      [12] = ground (0V)
  //      [13] = reference voltage (= ADC maximized)

  int adc_dout;                      //Voltage monitor ADC serial data receive
  int adc_sclock;                    //Voltage monitor ADC serial clock
  int adc_din;                       //Voltage monitor ADC serial data transmit
  int adc_cs;                        //Voltage monitor ADC chip select

  int adc_shiftin;
  int iclk;

  int write_data, read_data;

  for (int ich=0; ich<=14; ich++){
    adc_dout = 0;

    adc_din    = 0;
    adc_sclock = 0;
    adc_cs     = 1;

    write_data = 0;
    write_data |= (adc_sclock << 6);  
    write_data |= (adc_din    << 7);  
    write_data |= (adc_cs     << 8);  

    tmb_->WriteRegister(vme_adc_adr,write_data);

    adc_shiftin = ich << 4;      //d[7:4]=channel, d[3:2]=length, d[1:0]=ldbf,bip
    if (ich >= 14) adc_shiftin = 0;  //don't send channel 14, it is power-down

    //put adc_shiftin serially in 11 vme writes
    for (iclk=0; iclk<=11; iclk++){

      if (iclk <= 7) {
	adc_din = (adc_shiftin >> (7-iclk)) & 0x1;
      } else {
	adc_din = 0;
      }
      adc_sclock = 0;
      adc_cs     = 0;

      write_data = 0;
      write_data |= (adc_sclock << 6);  
      write_data |= (adc_din    << 7);  
      write_data |= (adc_cs     << 8);  
      
      tmb_->WriteRegister(vme_adc_adr,write_data);

      adc_sclock = 1;
      adc_cs     = 0;

      write_data = 0;
      write_data |= (adc_sclock << 6);  
      write_data |= (adc_din    << 7);  
      write_data |= (adc_cs     << 8);  
      
      tmb_->WriteRegister(vme_adc_adr,write_data);

      read_data = (tmb_->PowerComparator() >> 5) & 0x1;

      //pack output into adc_dout
      adc_dout |= (read_data << (11-iclk));
    }

    adc_din    = 0;
    adc_sclock = 0;
    adc_cs     = 1;

    write_data = 0;
    write_data |= (adc_sclock << 6);  
    write_data |= (adc_din    << 7);  
    write_data |= (adc_cs     << 8);  

    tmb_->WriteRegister(vme_adc_adr,write_data);

    if (ich>=1) {
      voltage[ich-1] = ((float) adc_dout / 4095.)*4.095; //convert adc value to volts
    }

  }

  voltage[0] *= 2.0;                      // 1V/2V
  voltage[5] /= 0.2;                      // 200mV/Amp
  voltage[6] /= 0.2;                      // 200mV/Amp
  voltage[7] /= 0.2;                      // 200mV/Amp
  voltage[8] /= 0.2;                      // 200mV/Amp
  voltage[9] /= 0.2;                      // 200mV/Amp if SH921 set 1-2, else comment out line

  return;
}
//////////////////////////////////////////
// END: The following should be in TMB: //
//////////////////////////////////////////
