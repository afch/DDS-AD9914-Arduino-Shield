#include "Menu.h"

void ClockFreqClass::incValue(int addendum)
{
    incrementValue(addendum);
    DDSCoreClock.SetNForDDSCoreFreq(BASE_DDS_CORE_CLOCK);
    DDSCoreClock.SetDDSCoreClock();
}

void ClockFreqClass::decValue(int addendum)
{
    decrementValue(addendum);
    DDSCoreClock.SetNForDDSCoreFreq(BASE_DDS_CORE_CLOCK);
    DDSCoreClock.SetDDSCoreClock();
}

void ClockSrcClass::incValue(int addendum)
{
  incrementValue(1);
  if ((value == TCXO_OCXO) || (value == EXT_TCXO_OCXO))
  {
    nextItem=&ClockFreq;
    CoreClockSave.prevItem=&DDSCoreClock;
  } else
    {
      nextItem=&ExtClockFreqGHz;
      CoreClockSave.prevItem=&ExtClockFreqMHz;
    }
}

void ClockSrcClass::decValue(int addendum)
{
  decrementValue(1);
  if ((value == TCXO_OCXO) || (value == EXT_TCXO_OCXO))
  {
    nextItem=&ClockFreq;
    CoreClockSave.prevItem=&DDSCoreClock;
  } else
    {
      nextItem=&ExtClockFreqGHz;
      CoreClockSave.prevItem=&ExtClockFreqMHz;
    }
}

void ExtClockFreqGHzClass::incValue(int addendum)
{
  incrementValue(addendum);
  //DDSCoreClock.DDSCoreClock=value*1000000000 + ExtClockFreqMHz.value*1000000;
  if ((value==ExtGHzMax) && ExtClockFreqMHz.value>ExtMHzMax) ExtClockFreqMHz.value=ExtMHzMax; 
  if ((value==ExtGHzMin) && ExtClockFreqMHz.value<ExtMHzMax) ExtClockFreqMHz.value=ExtMHzMax; 
}

void ExtClockFreqGHzClass::decValue(int addendum)
{
  decrementValue(addendum);
  //DDSCoreClock.DDSCoreClock=value*1000000000 + ExtClockFreqMHz.value*1000000;
  if ((value==ExtGHzMax) && ExtClockFreqMHz.value>ExtMHzMax) ExtClockFreqMHz.value=ExtMHzMax; 
  if ((value==ExtGHzMin) && ExtClockFreqMHz.value<ExtMHzMax) ExtClockFreqMHz.value=ExtMHzMax; 
}

void ExtClockFreqMHzClass::incValue(int addendum)
{
  incrementValue(addendum);
  //DDSCoreClock.DDSCoreClock=ExtClockFreqGHz.value*1000000000 + value*1000000;
  if ((value>ExtMHzMin) && (ExtClockFreqGHz.value==ExtGHzMax)) ExtClockFreqGHz.value=ExtGHzMax-1;
  if ((value<ExtMHzMin) && (ExtClockFreqGHz.value==ExtGHzMin)) ExtClockFreqGHz.value=ExtGHzMin+1;
}

void ExtClockFreqMHzClass::decValue(int addendum)
{
  decrementValue(addendum);
  //DDSCoreClock.DDSCoreClock=ExtClockFreqGHz.value*1000000000 + value*1000000;
  if ((value>ExtMHzMin) && (ExtClockFreqGHz.value==ExtGHzMax)) ExtClockFreqGHz.value=ExtGHzMax-1;;
  if ((value<ExtMHzMin) && (ExtClockFreqGHz.value==ExtGHzMin)) ExtClockFreqGHz.value=ExtGHzMin+1;;
}

bool DDSCoreClockClass::ExtClockFreqInRange()
{
  if (((ExtClockFreqGHz.value*1000000000ULL + ExtClockFreqMHz.value*1000000) < MIN_DDS_CORE_CLOCK) || ((ExtClockFreqGHz.value*1000000000ULL + ExtClockFreqMHz.value*1000000) > MAX_DDS_CORE_CLOCK)) return false;
    else return true;
}

uint64_t DDSCoreClockClass::GetDDSCoreClock()
{
  if ((ClockSrc.value == TCXO_OCXO) || (ClockSrc.value == EXT_TCXO_OCXO))
  {
    if (!IsFreqInRange()) SetDDSCoreClock();
    return DDSCoreClock;
  } else 
    {
      if (ExtClockFreqInRange())
        return ExtClockFreqGHz.value*1000000000ULL + ExtClockFreqMHz.value*1000000;
      else return BASE_EXT_DDS_CORE_CLOCK;
    }
}

void MenuLinking()
{
  // ** Main menu items linkning **
  // ** Next Items**
  GHZ.nextItem = &MHZ;
  MHZ.nextItem = &KHZ;
  KHZ.nextItem = &HZ;
  HZ.nextItem = &Amplitude;
  Amplitude.nextItem = &GHZ; //loop back to GHZ item
  // **Prev Items**
  Amplitude.prevItem = &HZ;
  HZ.prevItem = &KHZ;
  KHZ.prevItem = &MHZ;
  MHZ.prevItem = &GHZ;
  GHZ.prevItem = &Amplitude; //loop back to Amplitude item
  ////////

  // ** Core clock menu items linkning **
  // ** Next Items**
  ClockSrc.nextItem=&ClockFreq;
  ClockFreq.nextItem=&DDSCoreClock;
  DDSCoreClock.nextItem=&CoreClockSave;
  CoreClockSave.nextItem=&CoreClockExit;
  CoreClockExit.nextItem=&ClockSrc; //loop back to ClockSrc item

  ExtClockFreqGHz.nextItem=&ExtClockFreqMHz;
  ExtClockFreqMHz.nextItem=&CoreClockSave;
  
  // ** Prev Items**
  CoreClockExit.prevItem=&CoreClockSave;
  CoreClockSave.prevItem=&DDSCoreClock;
  DDSCoreClock.prevItem=&ClockFreq;
  ClockFreq.prevItem=&ClockSrc;
  ClockSrc.prevItem=&CoreClockExit; //loop back to CoreClockExit item

  ExtClockFreqMHz.prevItem=&ExtClockFreqGHz;
  ExtClockFreqGHz.prevItem=&ClockSrc;
}

void InitMenuItems()
{
  GHZ.maxValue=1;
  GHZ.minValue=0;
  
  MHZ.maxValue=999;
  MHZ.minValue=0;
  //MHZ.value=100;
  
  KHZ.maxValue=999;
  KHZ.minValue=0;

  HZ.maxValue=999;
  HZ.minValue=0;

  Amplitude.maxValue=68;
  Amplitude.minValue=0;

  ClockSrc.maxValue=EXT_OSC; //2
  ClockSrc.minValue=TCXO_OCXO; //0
  
  ClockFreq.maxValue=7;
  ClockFreq.minValue=0;

  //ClockFreq.value=4; //40mhz

  //KHZ.value=LOW_FREQ_LIMIT/1000;
  ui32CurrentOutputFreq=GetFreq();
  
  DDSCoreClock.DDSCoreClock=DDS_Core_Clock;
  DDSCoreClock.minValue=MIN_DDS_CORE_CLOCK_PLL;
  DDSCoreClock.maxValue=MAX_DDS_CORE_CLOCK_PLL;
  DDSCoreClock.minValue=N_MIN_VALUE;
  DDSCoreClock.maxValue=N_MAX_VALUE;

  //***********
  if (DDSCoreClock.value > N_MAX_VALUE) DDSCoreClock.value=DDSCoreClock.FindMinimalN();
  if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 > MAX_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMinimalN();

  if (DDSCoreClock.value < N_MIN_VALUE) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
  if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 < MIN_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
  //**********
  DDSCoreClock.SetDDSCoreClock();

  ExtClockFreqGHz.minValue=0;
  ExtClockFreqGHz.maxValue=4;

  ExtClockFreqMHz.minValue=0;
  ExtClockFreqMHz.maxValue=999;

  //BASE_EXT_DDS_CORE_CLOCK
  ExtClockFreqGHz.value=BASE_EXT_DDS_CORE_CLOCK/1000000000ULL; //4del
  ExtClockFreqMHz.value=(BASE_EXT_DDS_CORE_CLOCK%1000000000)/1000000; //4del
}

bool CoreClockSaveClass::goToEditMode (bool editMode)
    {
      
      if ((ClockSrc.value == TCXO_OCXO) || (ClockSrc.value == EXT_TCXO_OCXO)) DDS.initialize(DDSCoreClock.GetDDSCoreClock(), DDSCoreClock.value);
        else DDS.initialize(DDSCoreClock.GetDDSCoreClock(), 0);

      DDS.enableProfileMode();
      DDS.enableOSK();

      DDS.setFreq(GetFreq(),0);
            
      DDS.setAmpdB(0,0);

      SaveClockSettings();
      curItem=&CoreClockExit;
      return false;
    }


void LoadMainSettings()
{
  if (EEPROM.read(MAIN_SETTINGS_FLAG_ADR)!=55)
  {  
    GHZ.value=INIT_G;
    MHZ.value=INIT_M;
    KHZ.value=INIT_K;
    HZ.value=INIT_H;
    Amplitude.value=INIT_A;
    SaveMainSettings();
    
    //***********
    if (DDSCoreClock.value > N_MAX_VALUE) DDSCoreClock.value=DDSCoreClock.FindMinimalN();
    if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 > MAX_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMinimalN();

    if (DDSCoreClock.value < N_MIN_VALUE) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
    if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 < MIN_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
    //**********
    DDSCoreClock.SetDDSCoreClock();
    #if DBG==1
    Serial.println(F("Loading init values"));
    Serial.print("G=");
    Serial.println(GHZ.value);
    Serial.print("M=");
    Serial.println(MHZ.value);
    Serial.print("K=");
    Serial.println(KHZ.value);
    Serial.print("H=");
    Serial.println(HZ.value);
    Serial.print("A=");
    Serial.println(Amplitude.value);
    #endif
  }
  else
  {
    EEPROM.get(G_ADR, GHZ.value);
    EEPROM.get(M_ADR, MHZ.value);
    EEPROM.get(K_ADR, KHZ.value);
    EEPROM.get(H_ADR, HZ.value);
    EEPROM.get(A_ADR, Amplitude.value);
    
    #if DBG==1
    Serial.println(F("Value from EEPROM"));
    Serial.print("G=");
    Serial.println(GHZ.value);
    Serial.print("M=");
    Serial.println(MHZ.value);
    Serial.print("K=");
    Serial.println(KHZ.value);
    Serial.print("H=");
    Serial.println(HZ.value);
    Serial.print("A=");
    Serial.println(Amplitude.value);
    #endif
  }
}

void SaveMainSettings()
{
  EEPROM.put(G_ADR, GHZ.value);
  EEPROM.put(M_ADR, MHZ.value);
  EEPROM.put(K_ADR, KHZ.value);
  EEPROM.put(H_ADR, HZ.value);
  EEPROM.put(A_ADR, Amplitude.value);
  EEPROM.write(MAIN_SETTINGS_FLAG_ADR,55);
}


void LoadClockSettings()
{
  if (EEPROM.read(CLOCK_SETTINGS_FLAG_ADR)!=55)
  {
    ClockSrc.value=INIT_CLOCK_SOURCE_INDEX;
    ClockFreq.value=INIT_CLOCK_FREQ_INDEX;
    ExtClockFreqGHz.value=INIT_EXT_CLOCK_FREQ_GHZ;
    ExtClockFreqMHz.value=INIT_EXT_CLOCK_FREQ_MHZ;
    DDSCoreClock.value=INIT_DDS_CORE_CLOCK_N;
    #if DBG==1
    Serial.println(F("Set default clock's settings"));
    Serial.print(F("ClockSrc.value="));
    Serial.println(ClockSrc.value);
    Serial.print(F("ClockFreq.value="));
    Serial.println(ClockFreq.value);
    Serial.print(F("ExtClockFreqGHz.value="));
    Serial.println(ExtClockFreqGHz.value);
    Serial.print(F("ExtClockFreqMHz.value="));
    Serial.println(ExtClockFreqMHz.value);
    #endif

    SaveClockSettings();
  } else
  {
    EEPROM.get(CLOCK_SOURCE_ADR, ClockSrc.value);
    EEPROM.get(CLOCK_TCXO_FREQ_INDEX_ADR, ClockFreq.value);
    EEPROM.get(EXT_CLOCK_FREQ_GHZ_ADR, ExtClockFreqGHz.value);
    EEPROM.get(EXT_CLOCK_FREQ_MHZ_ADR, ExtClockFreqMHz.value);
    EEPROM.get(DDS_CORE_CLOCK_N_ADR, DDSCoreClock.value);

    #if DBG==1
    Serial.println(F("Reading clock's settings from EEPROM"));
    Serial.print(F("ClockSrc.value="));
    Serial.println(ClockSrc.value);
    Serial.print(F("ClockFreq.value="));
    Serial.println(ClockFreq.value);
    Serial.print(F("ExtClockFreqGHz.value="));
    Serial.println(ExtClockFreqGHz.value);
    Serial.print(F("ExtClockFreqMHz.value="));
    Serial.println(ExtClockFreqMHz.value);
    #endif
  }

  selectClockSrcPath(ClockSrc.value);

  //***********
  if (DDSCoreClock.value > N_MAX_VALUE) DDSCoreClock.value=DDSCoreClock.FindMinimalN();
  if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 > MAX_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMinimalN();

  if (DDSCoreClock.value < N_MIN_VALUE) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
  if (ClockFreq.Ref_Clk[ClockFreq.value]*(DDSCoreClock.value)*2 < MIN_DDS_CORE_CLOCK_PLL) DDSCoreClock.value=DDSCoreClock.FindMaximalN();
  //**********
  DDSCoreClock.SetDDSCoreClock();

  if ((ClockSrc.value == TCXO_OCXO) || (ClockSrc.value == EXT_TCXO_OCXO))
  {
    ClockSrc.nextItem=&ClockFreq;
    CoreClockSave.prevItem=&DDSCoreClock;
  } else
    {
      ClockSrc.nextItem=&ExtClockFreqGHz;
      CoreClockSave.prevItem=&ExtClockFreqMHz;
    }
}

void SaveClockSettings()
{
    EEPROM.put(CLOCK_SOURCE_ADR, ClockSrc.value);
    EEPROM.put(CLOCK_TCXO_FREQ_INDEX_ADR, ClockFreq.value);
    EEPROM.put(EXT_CLOCK_FREQ_GHZ_ADR, ExtClockFreqGHz.value);
    EEPROM.put(EXT_CLOCK_FREQ_MHZ_ADR, ExtClockFreqMHz.value);
    EEPROM.put(DDS_CORE_CLOCK_N_ADR, DDSCoreClock.value);
    
    EEPROM.write(CLOCK_SETTINGS_FLAG_ADR, 55);

    LoadClockSettings();
    
    //DisplayMessage("SETUP", "SAVED");
    DisplaySaved();
    delay(1000);
}

void DisplayMessage(String Title, String Message)
{
  display.clearDisplay();
  display.cp437(true); 
  display.setTextSize(2);      
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);
  display.print(Title);
  display.setTextSize(1); 
  display.setCursor(5, 28);
  display.print(Message);
  
  display.display();
}

void DisplaySaved(void)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40, 0);
  //display.setCursor(0, 0);
  display.cp437(true);
  display.println("SETUP");
  display.setCursor(40, 32);
  display.print("SAVED");
  display.display();
}
