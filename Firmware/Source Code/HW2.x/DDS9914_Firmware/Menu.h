#ifndef MENU_H
#define MENU_H

#define MAIN_MENU 0
#define CORE_CLOCK_MENU 1
uint8_t menuType = MAIN_MENU;
#define TCXO_OCXO 0


#define MIN_DDS_CORE_CLOCK_PLL 2400000000
#define MAX_DDS_CORE_CLOCK_PLL 2640000000
#define BASE_DDS_CORE_CLOCK 2500000000UL //это частота на которую будет сбрасываться значение DDS_Core_Clock в любых "непонятных" случаях

#define MAX_DDS_CORE_CLOCK 4500000000ULL //Hz (4.5GHz) максимально допустимое значение DDS_CORE_CLOCK
#define MIN_DDS_CORE_CLOCK 500000000 //Hz (500 MHz) минимально допустимое значение DDS_CORE_CLOCK
#define BASE_EXT_DDS_CORE_CLOCK 1000000000 //HZ (1 GHZ) //это частота на которую будет сбрасываться значение DDS_Core_Clock в режиме внешнего тактирования

#define N_MIN_VALUE 10
#define N_MAX_VALUE 255

const uint16_t ExtGHzMax=MAX_DDS_CORE_CLOCK/1000000000;
const uint16_t ExtGHzMin=MIN_DDS_CORE_CLOCK/1000000000;
const uint16_t ExtMHzMax=(MAX_DDS_CORE_CLOCK%1000000000)/1000000;
const uint16_t ExtMHzMin=MIN_DDS_CORE_CLOCK/1000000;


//**** EEPROM ADRESSES ****

#define CLOCK_SOURCE_ADR 0
#define CLOCK_TCXO_FREQ_INDEX_ADR 2
#define EXT_CLOCK_FREQ_GHZ_ADR 4
#define EXT_CLOCK_FREQ_MHZ_ADR 6
#define DDS_CORE_CLOCK_N_ADR 8

#define G_ADR 22
#define M_ADR 24
#define K_ADR 28
#define H_ADR 32
#define A_ADR 36

#define MAIN_SETTINGS_FLAG_ADR 100 // defualt settings 
#define CLOCK_SETTINGS_FLAG_ADR 101 // defualt clock settings flag address

//***********************

// **** INIT VALUES ******
#define INIT_G 0
#define INIT_M 100
#define INIT_K 0
#define INIT_H 0
#define INIT_A 0 

#define INIT_CLOCK_SOURCE_INDEX 0
#define INIT_CLOCK_FREQ_INDEX 4

#define INIT_EXT_CLOCK_FREQ_GHZ 3
#define INIT_EXT_CLOCK_FREQ_MHZ 0

#define INIT_DDS_CORE_CLOCK_N 30

//***********************

class MenuItem
{
  public:
    //String ItemName = "Default Menu Item Name";
    MenuItem* nextItem = NULL;
    MenuItem* prevItem = NULL;
    int16_t value = 0;
    int16_t maxValue = 0;
    int16_t minValue = 0;
    bool bBlink=false;
    MenuItem* moveToNextItem()
    {
      return nextItem;
    };
    MenuItem* moveToPrevItem()
    {
      return prevItem;
    };
    virtual void incValue(int addendum) = 0;
    virtual void decValue(int addendum) = 0;
    void incrementValue(int addendum)
    {
        value=value+abs(addendum);
        if (value > maxValue) value=minValue;
    }
    void decrementValue(int addendum)
    {
        value=value-abs(addendum);
        if (value < minValue) value=maxValue;
    }
    virtual bool goToEditMode(bool editMode)
    {
      //Blink=editMode;
      bBlink=!bBlink;
      return bBlink;
    }
    uint8_t GetColor()
    {
      static uint32_t lastTimeColorChangedTime=millis();
      static uint8_t color=1; //1 - white (normal color), 0 - black;
      if (bBlink==false) return 1;
      if ((millis()-lastTimeColorChangedTime)>333)
      {
        lastTimeColorChangedTime=millis();
        //color!=color;
        if (color==1) color=0; 
          else color=1;
      }
      return color;
    }
};

MenuItem* curItem;

// **************** MAIN MENU **********************************

class GHZClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      incrementValue(addendum);
    };
    void decValue(int addendum) {
      decrementValue(addendum);
    };
} GHZ;

class MHZClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      incrementValue(addendum);
    };
    void decValue(int addendum) {
      decrementValue(addendum);
    };
} MHZ;

class KHZClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      incrementValue(addendum);
    };
    void decValue(int addendum) {
      decrementValue(addendum);
    };
} KHZ;

class HZClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      incrementValue(addendum);
    };
    void decValue(int addendum) {
      decrementValue(addendum);
    };
} HZ;

class AmplitudeClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      //incrementValue();
      decrementValue(addendum);
    };
    void decValue(int addendum) {
      //decrementValue();
      incrementValue(addendum);
    };
} Amplitude;
// **************** MAIN MENU END **********************************

// **************** CLOCK SOURCE SETUP MENU ************************

class ClockSrcClass: public MenuItem
{
  public:
    void incValue(int addendum);
    void decValue(int addendum);
    String ClockSourceNames[2] = {"TCXO/OCXO", "Ext. Clock"};
} ClockSrc;

class ClockFreqClass: public MenuItem
{
  public:
    void incValue(int addendum);
    void decValue(int addendum);
    uint32_t Ref_Clk [8] = {5000000, 10000000, 20000000, 25000000, 40000000, 50000000, 100000000, 125000000};
} ClockFreq;

class DDSCoreClockClass: public MenuItem
{
  public:
    uint32_t DDSCoreClock=0;
    void incValue(int addendum) 
    {
      value++;
      if (value > N_MAX_VALUE) value=FindMinimalN();
      if (ClockFreq.Ref_Clk[ClockFreq.value]*(value)*2 > MAX_DDS_CORE_CLOCK_PLL) value=FindMinimalN();
      //DDSCoreClock=ClockFreq.Ref_Clk[ClockFreq.value]*value*2;
      SetDDSCoreClock();
      Serial.print("N=");
      Serial.println(value);
    }
    
    void decValue(int addendum) 
    {
      value--;
      if (value < N_MIN_VALUE) value=FindMaximalN();
      if (ClockFreq.Ref_Clk[ClockFreq.value]*(value)*2 < MIN_DDS_CORE_CLOCK_PLL) value=FindMaximalN();
      //DDSCoreClock=ClockFreq.Ref_Clk[ClockFreq.value]*value*2;
      SetDDSCoreClock();
      Serial.print("N=");
      Serial.println(value);
      
    }
    
    uint16_t FindMinimalN()
    {
      uint16_t tmpN=ceil(MIN_DDS_CORE_CLOCK_PLL/2/ClockFreq.Ref_Clk[ClockFreq.value]);
      if (tmpN < N_MIN_VALUE) tmpN=N_MIN_VALUE;
      return tmpN;
    }
    
    uint16_t FindMaximalN()
    {
      uint16_t tmpN=floor(MAX_DDS_CORE_CLOCK_PLL/2/ClockFreq.Ref_Clk[ClockFreq.value]);
      if (tmpN > N_MAX_VALUE) tmpN=N_MAX_VALUE;
      return tmpN;
    }
    
    void SetNForDDSCoreFreq(uint32_t ui32Freq) // Set N (value) to get nearest value of freq. 
    {
      uint16_t tmpN=ui32Freq/2/ClockFreq.Ref_Clk[ClockFreq.value];
      if (tmpN < N_MIN_VALUE) tmpN=N_MIN_VALUE;
      if (tmpN > N_MAX_VALUE) tmpN=N_MAX_VALUE;
      //return tmpN;
      value=tmpN;
    }
    
    void SetDDSCoreClock()
    {
      DDSCoreClock=ClockFreq.Ref_Clk[ClockFreq.value]*value*2;
    }

    bool IsFreqInRange()
    {
      if ((DDSCoreClock>MIN_DDS_CORE_CLOCK_PLL) && (DDSCoreClock<MAX_DDS_CORE_CLOCK_PLL)) return true;
      else return false;
    }
    
    uint64_t GetDDSCoreClock();

    DDSCoreClockClass()
    {
      SetNForDDSCoreFreq(BASE_DDS_CORE_CLOCK);
      SetDDSCoreClock();
    }

    bool ExtClockFreqInRange();
    
} DDSCoreClock;

class CoreClockSaveClass: public MenuItem
{
  public:
    void incValue(int addendum) {
      
    };
    
    void decValue(int addendum) {
      
    };
    
    bool goToEditMode (bool editMode);
} CoreClockSave;

class CoreClockExitClass: public MenuItem
{
  public:
    void incValue(int addendum) {
     
    };
    void decValue(int addendum) {
      ;
    };
    bool goToEditMode (bool editMode)
    {
      //Serial.println("Exit!!!");
      curItem = &GHZ;
      menuType = MAIN_MENU;
      return false;
    }
} CoreClockExit;

class ExtClockFreqGHzClass: public MenuItem
{
  public:
    void incValue(int addendum);
    void decValue(int addendum);
   
} ExtClockFreqGHz;

class ExtClockFreqMHzClass: public MenuItem
{
  public:
    void incValue(int addendum);
    void decValue(int addendum);
   
} ExtClockFreqMHz;


void MenuLinking();
void InitSettings();

void LoadMainSettings();
void SaveMainSettings();

void LoadClockSettings();
void SaveClockSettings();

void DisplayMessage(String Title, String Message);
void DisplaySaved(void);

#endif
