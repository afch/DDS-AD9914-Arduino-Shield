/*
   В проекте была использована плата AD9914/5 v2.0 https://gra-afch.com/product-category/rf-units/
   AD9914 and AD9915 - эти ДДС одинаковые за исключением частоты тактирования и битов [8:9] в регистре CFR2(0x01)
   //**********Warning**********
   - Чтобы на выходе получить 0dBm, ставить R1 и R3 НЕ нужно!!! так как в документации Analog Devicec написано -
   "AOUT(пин 42), /AOUT(41) Internally connected through a 50 Ohm resistor to AVDD (3.3 V)". 
   Но если поставить резисторов R1 и R3 по 50 Ohm, то выходной уровень будет -3 Дбм.
   Выходной трансорматор T1 Mini-Circuits TC1-1-13M+, decoupled capacitors C18, C19 - 100nF 0603.
   
   - В цепи тактирования AD9914/AD9915 (pin 54 /REF_CLK и pin 55 REF_CLK)можно использовать как электронный 
   ADCLK905/925 на плате AD9914/5 v1.1, 
   a так же можно использовать на плате AD9914/5 v2.0 RF Transformator Balun например 617DB-1022/ JA4220-AL

   - Если используеться внтуренрий PLL In manual mode, we managed to run only at a frequency of 2.4 GHz
    for automatic mode, it is very important to first calibrate the Charge Pump Current - VCO_Cal_enable, and then DAC_CAL_enable.
    As a result of the experiment, we found out that AD9915 VCO (PLL) operates in the range from 2150 to 2640 MHz.

   www.gra-afch.com 

  За снову мы взяли библиотеку автора Ben Reschovsky, и модифицировали ее авторы Alec Fomin and Grisha Anofriev. 
  Добавленны функции с работой PLL и калибровки VCO.   
 */


#include "Arduino.h"
#include "SPI.h"
#include "AD9914.h"
#include <math.h>


/* CONSTRUCTOR */

// Constructor function; initializes communication pinouts
AD9914::AD9914(byte ssPin, byte resetPin, byte updatePin, byte ps0, byte ps1, byte ps2, byte osk) //Master_reset pin?
{
    RESOLUTION  = 4294967296.0;
    _ssPin = ssPin;
    _resetPin = resetPin;
    _updatePin = updatePin;
    _ps0 = ps0;
    _ps1 = ps1;
    _ps2 = ps2;
    _osk = osk;


    // sets up the pinmodes for output
    pinMode(_ssPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_updatePin, OUTPUT);
    pinMode(_ps0, OUTPUT);
    pinMode(_ps1, OUTPUT);
    pinMode(_ps2, OUTPUT);
    pinMode(_osk, OUTPUT);

    // defaults for pin logic levels
    digitalWrite(_ssPin, HIGH);
    digitalWrite(_resetPin, LOW);
    digitalWrite(_updatePin, LOW);
    digitalWrite(_ps0, LOW);
    digitalWrite(_ps1, LOW);
    digitalWrite(_ps2, LOW);
    digitalWrite(_osk, LOW);
}

    /* PUBLIC CLASS FUNCTIONS */


// initialize(refClk) - initializes DDS with reference clock frequency refClk (assumes you are using an external clock, not the internal PLL)
void AD9914::initialize(unsigned long refClk, uint8_t N){
    _refIn = refClk;
    _refClk = refClk;

    AD9914::reset();
    
    delay(100);
    
    _profileModeOn = false; //profile mode is disabled by default
    _OSKon = false; //OSK is disabled by default
    
    _activeProfile = 0; 
    
    /***PLL *
    * In manual mode, we managed to run only at a frequency of 2.4 GHz
    * for automatic mode, it is very important to first calibrate the Charge Pump Current - VCO_Cal_enable, and then DAC_CAL_enable.
    * As a result of the experiment, we found out that ad9915 VCO (PLL) operates in the range from 2150 to 2640 MHz.
    ****/
    /****Manual_ICP_selection***/
   /* byte registerInfo[] = {CFR3, 4};
    byte data[] = {0x00, 
                  PLL_enable, 
                   N, //32,  // N Value PLLMultiplier, PLL_OUT = IN_REF_CLK x 2 x N
                   Manual_ICP_selection | ICP_500uA};// //[31:0]
    AD9914::writeRegister(registerInfo, data);
    AD9914::dacCalibrate(); //Calibrate DAC !!!!!!!!!!!!!!!!!!!!!!!ALEC
    */
     /**** value is automatically chosen via the VCO calibration process***/
    enableOSK();
    if (N)
    {
    byte registerInfo[] = {CFR3, 4};
    byte data[] = {0x00, 
                  PLL_enable, 
                   N, //32,  // N Value PLLMultiplier, PLL_OUT = IN_REF_CLK x 2 x N
                   0};// //[31:0]
    AD9914::writeRegister(registerInfo, data);
    AD9914::dacCalibrate(); //Calibrate DAC !!!!!!!!!!!!!!!!!!!!!!!ALEC
    } else 
    {
       byte registerInfo[] = {CFR3, 4};
       byte data[] = {0, 0, 0, 0}; //[31:0]
       AD9914::writeRegister(registerInfo, data);
    }
    
/*
   // Disable PLL GRISHA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    byte registerInfo[] = {CFR3, 4};
    byte data[] = {0, 0, 0, 0}; //[31:0]
    AD9914::writeRegister(registerInfo, data);
    */
    

    //gra & afch
    /*registerInfo[] = {0x01, 4};
    data[] = {0x00, 0x00, 0x09, 0x00};
    AD9914::writeRegister(registerInfo, data);*/

}

// reset() - takes no arguments; resets DDS
void AD9914::reset(){
    digitalWrite(_resetPin, HIGH);
    delay(1);
    digitalWrite(_resetPin, LOW);
}

// update() - sends a logic pulse to IO UPDATE pin on DDS; updates frequency output to 
//      newly set frequency (FTW0)
void AD9914::update(){
    digitalWrite(_updatePin, HIGH);
    delay(1);
    digitalWrite(_updatePin, LOW);
}

// setFreq(freq) -- writes freq to DDS board, in FTW0
void AD9914::setFreq(unsigned long freq, byte profile){
    
    if (profile > 7) {
        return; //invalid profile, return without doing anything
    } 
   
    // set _freq and _ftw variables
    _freq[profile] = freq;
    _ftw[profile] = round(freq * RESOLUTION / _refClk) ;

    // divide up ftw into four bytes
    byte ftw[] = { lowByte(_ftw[profile] >> 24), lowByte(_ftw[profile] >> 16), lowByte(_ftw[profile] >> 8), lowByte(_ftw[profile])};
    // register info -- writing four bytes to register 0x04, 
    
    byte registerInfo[] = {PROFILE0_REGISTER, 4};
    
//    else if (profile == 0) {  //select the right register for the commanded profile number
//      registerInfo[0]=0x0B;
//    } else if (profile == 1) {
//      registerInfo[0]=0x0D;
//    } else if (profile == 2) {
//      registerInfo[0]=0x0F;
//    } else if (profile == 3) {
//      registerInfo[0]=0x11;
//    } else if (profile == 4) {
//      registerInfo[0]=0x13;
//    } else if (profile == 5) {
//      registerInfo[0]=0x15;
//    } else if (profile == 6) {
//      registerInfo[0]=0x17;
//    } else if (profile == 7) {
//      registerInfo[0]=0x19;
//    }
    registerInfo[0] += 2*profile; //select the right register for the commanded profile number
   
    
    
    //byte CFR1[] = { 0x00, 0x00, 0x00, 0x00 };
    //byte CFR1Info[] = {0x00, 4};
    
	// actually writes to register
    //AD9914::writeRegister(CFR1Info, CFR1);

    AD9914::writeRegister(registerInfo, ftw);
    
	// issues update command
    AD9914::update();
}
void AD9914::setFreq(unsigned long freq){
   AD9914::setFreq(freq,0); 
}

void AD9914::setAmp(double scaledAmp, byte profile){
   if (profile > 7) {
        return; //invalid profile, return without doing anything
   } 
   
   _scaledAmp[profile] = scaledAmp;
   _asf[profile] = round(scaledAmp*4096);
   _scaledAmpdB[profile] = 20.0*log10(_asf[profile]/4096.0);
   
   if (_asf[profile] >= 4096) {
      _asf[profile]=4095; //write max value
   } else if (scaledAmp < 0) {
      _asf[profile]=0; //write min value
   }
   
   AD9914::writeAmp(_asf[profile],profile);

   AD9914::update(); //Alec
}
       
void AD9914::setAmp(double scaledAmp){
  AD9914::setAmp(scaledAmp,0);
}

void AD9914::setAmpdB(double scaledAmpdB, byte profile){
  if (profile > 7) {
        return; //invalid profile, return without doing anything
   } 
   
   if (scaledAmpdB > 0) {
       return; //only valid for attenuation, so dB should be less than 0, return without doing anything
   }
   
   //_scaledAmpdB[profile] = scaledAmpdB;
   _asf[profile] = round(pow(10, scaledAmpdB/20.0)*4096.0);
   //_scaledAmp[profile] = _asf[profile]/4096.0;

   if (_asf[profile] >= 4096) {
      _asf[profile]=4095; //write max value
   }

   AD9914::writeAmp(_asf[profile],profile); 
}

void AD9914::setAmpdB(double scaledAmpdB){
  AD9914::setAmpdB(scaledAmpdB,0);
}

//Gets current amplitude
double AD9914::getAmp(byte profile){
  return _scaledAmp[profile];
}
double AD9914::getAmp(){
  return _scaledAmp[0];
}
        
// Gets current amplitude in dB
double AD9914::getAmpdB(byte profile){
  return _scaledAmpdB[profile];
}
double AD9914::getAmpdB(){
  return _scaledAmpdB[0];
}
        
//Gets current amplitude tuning word
unsigned long AD9914::getASF(byte profile){
  return _asf[profile];
}
unsigned long AD9914::getASF(){
  return _asf[0];
}



// getFreq() - returns current frequency
unsigned long AD9914::getFreq(byte profile){
    return _freq[profile];
}

// getFreq() - returns frequency from profile 0
unsigned long AD9914::getFreq(){
    return _freq[0];
}

// getFTW() -- returns current FTW
unsigned long AD9914::getFTW(byte profile){
    return _ftw[profile];
}

unsigned long AD9914::getFTW(){
    return _ftw[0];
}

// Function setFTW -- accepts 32-bit frequency tuning word ftw;
//      updates instance variables for FTW and Frequency, and writes ftw to DDS.
void AD9914::setFTW(unsigned long ftw, byte profile){
  
    if (profile > 7) {
        return; //invalid profile, return without doing anything
    } 
    
    // set freqency and ftw variables
    _ftw[profile] = ftw;
    _freq[profile] = ftw * _refClk / RESOLUTION;

    // divide up ftw into four bytes
    byte data[] = { lowByte(ftw >> 24), lowByte(ftw >> 16), lowByte(ftw >> 8), lowByte(ftw)};
    // register info -- writing four bytes to register 0x04, 

    byte registerInfo[] = {0x0B, 4};
    registerInfo[0] += 2*profile; //select the right register for the commanded profile number

	
    //byte CFR1[] = { 0x00, 0x00, 0x00, 0x00 };
    //byte CFR1Info[] = {0x00, 4};
	
    //AD9914::writeRegister(CFR1Info, CFR1);
    AD9914::writeRegister(registerInfo, data);
    AD9914::update();

}

void AD9914::setFTW(unsigned long ftw){
  AD9914::setFTW(ftw,0);
}

//Enable the profile select mode
void AD9914::enableProfileMode() {
  //write 0x01, byte 23 high
  _profileModeOn = true;
  byte registerInfo[] = {CFR2, 4};
  //byte data[] = {0x00, PROFILE_MODE_ENABLE, SYNC_CLK_ENABLE | SYNC_OUT_IN_MUX_ENABLE, 0x00};
  //byte data[] = {0x00, PROFILE_MODE_ENABLE,  SYNC_OUT_IN_MUX_ENABLE, 0x00}; // disable SYNC_CLK_ENABLE
  byte data[] = {0x00, PROFILE_MODE_ENABLE,  0, 0x00}; // 
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
}

//Disable the profile select mode
void AD9914::disableProfileMode() {
  //write 0x01, byte 23 low
  _profileModeOn = false;
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x00, 0x09, 0x00};
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
}

//enable OSK
void AD9914::enableOSK(){
  //write 0x00, byte 8 high
  _OSKon = true;
  byte registerInfo[] = {CFR1, 4};
// byte data[] = {0x00, ENABLE_SINE_OUTPUT, OSK_ENABLE, EXTERNAL_POWER_DOWN_CONTROL | SDIO_INPUT_ONLY}; // Alec when PLL Disabled
 byte data[] = {VCO_CAL_ENABLE, ENABLE_SINE_OUTPUT, OSK_ENABLE, EXTERNAL_POWER_DOWN_CONTROL | SDIO_INPUT_ONLY}; // Grisha when PLL Enabled VCO_CAL_ENABLE 
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();

  AD9914::dacCalibrate(); //Calibrate DAC
  AD9914::update();
}
        
//disable OSK
void AD9914::disableOSK(){
  //write 0x00, byte 8 low
  _OSKon = false;
  byte registerInfo[] = {0x00, 4};
  byte data[] = {0x00, 0x01, 0x00, 0x08};
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
}
  
//return boolean indicating if profile select mode is activated
boolean AD9914::getProfileSelectMode() {
  return _profileModeOn;
}

//return boolean indicating if OSK mode is activated
boolean AD9914::getOSKMode() {
  return _OSKon;
}

void AD9914::enableSyncClck() {
 //write 0x01, byte 11 high 
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x80, 0x09, 0x00};
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
}

void AD9914::disableSyncClck() {
  //write 0x01, byte 11 low
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x80, 0x01, 0x00};
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
}

void AD9914::selectProfile(byte profile){
  //Possible improvement: write PS pin states all at once using register masks
  _activeProfile = profile;
  
  if (profile > 7) {
    return; //not a valid profile number, return without doing anything
  }
  
  if ((B00000001 & profile) > 0) { //rightmost bit is 1
      digitalWrite(_ps0, HIGH);
  } else {
      digitalWrite(_ps0,LOW);
  }
  if ((B00000010 & profile) > 0) { //next bit is 1
      digitalWrite(_ps1, HIGH);
  } else {
      digitalWrite(_ps1,LOW);
  }
  if ((B00000100 & profile) > 0) { //next bit is 1
      digitalWrite(_ps2, HIGH);
  } else {
      digitalWrite(_ps2,LOW);
  }
  
}

byte AD9914::getProfile() {
  return _activeProfile;
}



/* PRIVATE CLASS FUNCTIONS */


// Writes SPI to particular register.
//      registerInfo is a 2-element array which contains [register, number of bytes]
void AD9914::writeRegister(byte registerInfo[], byte data[]){

    digitalWrite(_ssPin, LOW);

    // Writes the register value
    SPI.transfer(registerInfo[0]);
    /*Serial.print("RecVal=");
    Serial.println(RecVal, BIN);*/
    
    // Writes the data
    for(int i = 0; i < registerInfo[1]; i++)
    {
        SPI.transfer(data[i]);
    }

    digitalWrite(_ssPin, HIGH);

}

void AD9914::dacCalibrate(){
  //Calibrate DAC (0x03, bit 24 -> high then low)
  byte registerInfo[] = {CFR4, 4};
  byte data[] = {DAC_CAL_ENABLE, 0x05, 0x21, 0x20}; //write bit high
  AD9914::writeRegister(registerInfo, data);
  AD9914::update();
  delay(10);
  //delayMicroseconds(1035);
  data[0] = 0x00; //clear DAC_CAL_ENABLE
  AD9914::writeRegister(registerInfo, data); 
  AD9914::update();
}

void AD9914::writeAmp(long ampScaleFactor, byte profile){
  byte registerInfo[] = {PROFILE0_PHASE_AMPLITUDE, 4};
  
  registerInfo[0] += 2*profile; //select the right register for the commanded profile number
  
  // divide up ASF into two bytes, pad with 0s for the phase offset
  byte atw[] = {lowByte(ampScaleFactor >> 8), lowByte(ampScaleFactor), 0x00, 0x00};
  
  // actually writes to register
  AD9914::writeRegister(registerInfo, atw);
  
  AD9914::update();
  
}
