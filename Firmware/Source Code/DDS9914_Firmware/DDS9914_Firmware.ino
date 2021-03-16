#define DBG 0

#include <Adafruit_SSD1306.h>
#include "Menu.h"
#include "DisplayMenu.h"
#include <ClickButton.h>
//#include <Encoder_GA_E/Encoder.h>
#include <Encoder.h>
#include <SPI.h>
#include "AD9914.h"
#include <EEPROM.h>

#define FIRMWAREVERSION 0.8 //27.09.2020
//v.0.8 24.09.2020 Добавляем сохранение всех параметров в EEPROM
//v.0.7 23.09.2020 Внедряем управление DDS
//v.0.6 22.09.2020 Добавлены ограничение и проверка при установке внешней частоты тактирования
//v.0.5 21.09.2020 Почти додеално меню с настройками такитрования

//Declare the DDS object:
AD9914 DDS(CSPIN, RESETPIN, IO_UPDATEPIN, PS0PIN, PS1PIN, PS2PIN, OSKPIN);

uint32_t EXT_OSC_Freq=BASE_DDS_CORE_CLOCK;
uint32_t DDS_Core_Clock=BASE_DDS_CORE_CLOCK;
#define LOW_FREQ_LIMIT  100000 //Hz
//#define HIGH_FREQ_LIMIT  2250000000 //Hz HIGH_FREQ_LIMIT must be calcultaed as CoreClock/2
uint32_t ui32HIGH_FREQ_LIMIT=DDS_Core_Clock/2;
uint32_t ui32CurrentOutputFreq=0;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1, 800000UL, 800000UL);

#define MODE_PIN 2    //A0
//#define DOWN_PIN     A1
//#define UP_PIN       A2

ClickButton modeButton (MODE_PIN, LOW, CLICKBTN_PULLUP);
//ClickButton downButton (DOWN_PIN, LOW, CLICKBTN_PULLUP);
//ClickButton upButton (UP_PIN, LOW, CLICKBTN_PULLUP);

Encoder myEnc(19, 18);

ScrollingText stTooHighFreq("Too HIGH Freq. > "+String(trunc((ui32HIGH_FREQ_LIMIT)/float(1E8))/10.0, 1)+" GHz", 10, 150);
ScrollingText stTooLowFreq("Too LOW Freq. < "+String((LOW_FREQ_LIMIT)/float(1E3), 0)+" kHz", 10, 150);

void setup() {

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.cp437(true);
  display.clearDisplay();

  DisplayHello();
  delay(2000);

  Serial.begin(115200);

  Serial.println(F("DDS AD9914 Arduino Shield by GRA & AFCH. (gra-afch.com)"));
  Serial.print(F("Firmware v.:"));
  Serial.println(FIRMWAREVERSION);

  modeButton.Update();

  if (modeButton.depressed == true) //если при включении была зажата кнопка MODE, то затираем управляющие флаги в EEPROM, которые восстановят заводские значения всех параметров
  {
    EEPROM.write(CLOCK_SETTINGS_FLAG_ADR, 255); //flag that force save default clock settings to EEPROM 
    EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 255); //flag that force save default main settings to EEPROM 
    //EEPROM.write(MODULATION_SETTINGS_FLAG, 255); //flag that force save default modulation settings to EEPROM 
  }

  MenuLinking();
  InitMenuItems();
  
  LoadMainSettings();
  LoadClockSettings();

  ui32CurrentOutputFreq=GetFreq();

  curItem = &GHZ;

  //************ 4 del ***********
  //curItem = &ClockSrc; // 4 del
  //menuType = CORE_CLOCK_MENU; // 4 del
  //************ 4 del end ********

  modeButton.debounceTime   = 25;   // Debounce timer in ms
  modeButton.multiclickTime = 10;  // Time limit for multi clicks
  modeButton.longClickTime  = 1000; // time until "held-down clicks" register

  //********* DDS SETUP **************
  pinMode(F0, OUTPUT);
  pinMode(F1, OUTPUT);
  pinMode(F2, OUTPUT);
  pinMode(F3, OUTPUT);
  pinMode(EXT_PWR_DWN, OUTPUT);
  pinMode(SYNCIO_PIN, OUTPUT);

  pinMode(RESETPIN, OUTPUT);

  digitalWrite(F0, HIGH);
  digitalWrite(F1, LOW);
  digitalWrite(F2, LOW);
  digitalWrite(F3, LOW);

  digitalWrite(SYNCIO_PIN, LOW);
  
  digitalWrite(EXT_PWR_DWN, LOW);
  
  SPI.begin();
  SPI.setClockDivider(4);
  SPI.setDataMode(SPI_MODE0); //was 0
  SPI.setBitOrder(MSBFIRST);
  //*****************

  //DDS.initialize(2560000000); //Grisha
  DDS.initialize(DDSCoreClock.GetDDSCoreClock(), DDSCoreClock.value);
  DDS.enableProfileMode();
  DDS.enableOSK();

  //DDS.setFreq(100000000,0); 
  DDS.setFreq(GetFreq(),0);
  //DDS.setAmpdB(-10,0);
  DDS.setAmpdB(Amplitude.value, 0);
  DDS.selectProfile(0);

  pinMode(OSKPIN, OUTPUT);
  digitalWrite(OSKPIN, HIGH);
}

bool MenuEditMode=false;

long oldPosition  = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int curPos=0;
  curPos=myEnc.read();
  
  modeButton.Update();

  if (modeButton.clicks == 1) 
  {
    //MenuEditMode=!MenuEditMode;
    MenuEditMode=curItem->goToEditMode(MenuEditMode);
  }

  if ((modeButton.clicks == -1) && (modeButton.depressed == true)) 
  {
    curItem = &ClockSrc; 
    menuType = CORE_CLOCK_MENU; 
  }
  

  //long newPosition = curPos; //myEnc.read();///2;
  /*if (curPos != oldPosition) 
  {
    oldPosition = curPos;
    if (curPos!=0) Serial.println(curPos);
  }*/

  if (curPos>0)
  {
    //oldPosition = newPosition;
    switch(MenuEditMode)
    {
      case true: 
        curItem->incValue(curPos); 
        ui32CurrentOutputFreq=GetFreq(); 
        DDS.setFreq(ui32CurrentOutputFreq,0); 
        DDS.setAmpdB(Amplitude.value * -1, 0); 
        if (menuType == MAIN_MENU) SaveMainSettings();
      break;
      case false: curItem=curItem->moveToNextItem(); break;
    }
  }
  
  if (curPos<0)
  {
    switch(MenuEditMode)
    {
      case true: 
        curItem->decValue(curPos);
        ui32CurrentOutputFreq=GetFreq();
        DDS.setFreq(ui32CurrentOutputFreq,0);
        DDS.setAmpdB(Amplitude.value * -1, 0);
        if (menuType == MAIN_MENU) SaveMainSettings();
      break;
      case false: curItem=curItem->moveToPrevItem(); break;
    }
  }

  DisplayMenu(menuType); 

}

uint32_t GetFreq()
{
  return GHZ.value * 1000000000UL + MHZ.value * 1000000UL + KHZ.value * 1000UL + HZ.value;
}

int8_t FreqInRange() //1 - Higher, -1 - lower, 0 - in range
{
  //ui32HIGH_FREQ_LIMIT=DDS_Core_Clock/2;
  if (ui32CurrentOutputFreq>ui32HIGH_FREQ_LIMIT) return 1;
  if (ui32CurrentOutputFreq<LOW_FREQ_LIMIT) return -1;
  return 0;
}
