// *************** ONLY FOR HW v3.x ************
#define DBG 0

#include <Adafruit_SSD1306.h>
#include "Menu.h"
#include "DisplayMenu.h"
#include <ClickButton.h>
//#include <Encoder_GA_E/Encoder.h>
#include <Encoder.h>
#ifndef GRA_AND_AFCH_ENCODER_MOD3 
  #error The "Encoder" library v3 modified by GRA and AFCH must be used!
#endif
#include <SPI.h>
#include "AD9914.h"
#include <EEPROM.h>

#include <GParser.h>

#define FIRMWAREVERSION 0.89
//v0.89 26.12.2024 //доработано переключения источников тактирования: добавлен внешнй EXT_TCXO_OCXO, 
                   //добавлено управление индикатором защелки PLL
//v0.88 26.12.2024 //включение и выключение кнопкой
//v0.87 24.12.2024 //добавлено измерение входного напряжение и отключение если оно выходит за диапазон 10 - 14 вольт
//v0.86 23.12.2024 Начато внедрение новых функций для версии 3.х:
  // +коммутатор источников тактирования
  // +включение и выключение кнопкой
  // +измерение входного напряжения питания
  // +идикатор защелки PLL (+активируется CFR3[2], читается из 0x1B[24])
  // +нужно убрать минус в -00dBm
//v0.85 26.05.2024 
//добавлен вывод температуры на экран и выключение DDS при привышении заднного порога (99 градусов)
//v0.84 22.06.2023 
//Добавленны комнады вклчения и отключения выходов
//v0.83 20.06.2023
//ускорена обработка комманд переданных через последвоательный порт 
//v0.82 06.06.2023 
//Добавление поддержки комманд через последовательный порт
//Обновлена библиотека энкодера до версии 3, для более стабильной работы
//v0.81 08.04.2021 Обновление библиотеки Encoder
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

bool isPWR_DWN = false;

#define MODE_PIN     2    //A0
#define RFOUT_PIN    3
//#define DOWN_PIN     A1
//#define UP_PIN       A2

ClickButton modeButton (MODE_PIN, LOW, CLICKBTN_PULLUP);
ClickButton RFOutButton(RFOUT_PIN, LOW, CLICKBTN_PULLUP);
//ClickButton downButton (DOWN_PIN, LOW, CLICKBTN_PULLUP);
//ClickButton upButton (UP_PIN, LOW, CLICKBTN_PULLUP);

Encoder myEnc(19, 18);

ScrollingText stTooHighFreq("Too HIGH Freq. > "+String(trunc((ui32HIGH_FREQ_LIMIT)/float(1E8))/10.0, 1)+" GHz", 10, 150);
ScrollingText stTooLowFreq("Too LOW Freq. < "+String((LOW_FREQ_LIMIT)/float(1E3), 0)+" kHz", 10, 150);

#define TC1407_SENSOR_PIN A3
#define TC1407_READ_DELAY 100
#define TC1407_SAMPLES 100
#define OVER_TEMP_THR 99  //99

void setup() {

  analogReference(DEFAULT); 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.cp437(true);
  display.clearDisplay();

   Serial.begin(115200);

  Serial.println(F("DDS AD9914 Arduino Shield by GRA & AFCH. (gra-afch.com)"));
  Serial.print(F("Firmware v.:"));
  Serial.println(FIRMWAREVERSION);

  Serial.print(F("Input voltage = "));
  Serial.println(getInputVoltage());

  if ((getInputVoltage() < 10) ||
      (getInputVoltage() > 14)) //13
      {
        pinMode(EXT_PWR_DWN, OUTPUT);
        digitalWrite(EXT_PWR_DWN, HIGH);
        displayPowerWarning();
        while(1);
      }

  DisplayHello();
  delay(2000);

  modeButton.Update();

  if (modeButton.depressed == true) //если при включении была зажата кнопка MODE, то затираем управляющие флаги в EEPROM, которые восстановят заводские значения всех параметров
  {
    EEPROM.write(CLOCK_SETTINGS_FLAG_ADR, 255); //flag that force save default clock settings to EEPROM 
    EEPROM.write(MAIN_SETTINGS_FLAG_ADR, 255); //flag that force save default main settings to EEPROM 
    //EEPROM.write(MODULATION_SETTINGS_FLAG, 255); //flag that force save default modulation settings to EEPROM 
  }

  MenuLinking();
  InitMenuItems();

    //********** V3.x ********
  pinMode(EXTERANL_SRC_PATH_PIN, OUTPUT);
  pinMode(TCXO_PATH_PIN, OUTPUT);
  pinMode(TCXO_POWER_PIN, OUTPUT);
  pinMode(PLL_LOCK_LED_PIN, OUTPUT);
  //pinMode(INPUT_VOLTAGE_PIN, INPUT); // не уверен что это нужно

  digitalWrite(TCXO_POWER_PIN, LOW);
  digitalWrite(TCXO_PATH_PIN, LOW);
  digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);
  //************************
  
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

  RFOutButton.debounceTime   = 25;   // Debounce timer in ms
  RFOutButton.multiclickTime = 1;  // Time limit for multi clicks
  RFOutButton.longClickTime  = 1000; // time until "held-down clicks" register

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
  temperature = readTC1407(TC1407_SENSOR_PIN);
  if (temperature > OVER_TEMP_THR) ohp();

  ReadSerialCommands();
  int curPos=0;
  curPos=myEnc.read();
  
  modeButton.Update();
  RFOutButton.Update();

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
  
   if (RFOutButton.clicks > 0) // short click 
    {
      isPWR_DWN = !isPWR_DWN;
      if (isPWR_DWN) 
      {
        digitalWrite(EXT_PWR_DWN, HIGH);
      }
        else 
        {
          digitalWrite(EXT_PWR_DWN, LOW);
          DDS.initialize(DDSCoreClock.GetDDSCoreClock(), DDSCoreClock.value);
          DDS.enableProfileMode();
          DDS.enableOSK();
          DDS.setFreq(GetFreq(),0);
          DDS.setAmpdB(Amplitude.value, 0);
          Serial.println("ON");
        }
    }
  //long newPosition = curPos; //myEnc.read();///2;
  /*if (curPos != oldPosition) 
  {
    oldPosition = curPos;
    if (curPos!=0) Serial.println(curPos);
  }*/

  if (isPLLLocked()) digitalWrite(PLL_LOCK_LED_PIN, HIGH);
    else digitalWrite(PLL_LOCK_LED_PIN, LOW);

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

int8_t readTC1407(uint8_t pin)
{
  static uint32_t lastTimeRead = 0;
  static int8_t value = 25;
  static float ADCSum = 0;
  static uint8_t counter = 0;
  float voltage = 0;
  if (millis() - lastTimeRead > TC1407_READ_DELAY)
  {
    lastTimeRead = millis();
    ADCSum = ADCSum + analogRead(pin);
    //Serial.print("ADCSum=");
    //Serial.println(ADCSum);
    //Serial.print("analogRead(pin)=");
    //Serial.println(analogRead(pin));
    if (counter < TC1407_SAMPLES) counter++;
      else
      {
        counter = 0;
        voltage = map((ADCSum / TC1407_SAMPLES), 0, 1023, 0, 49500/*25600*/); //4.95
        /*Serial.print("ADCSum=");
        Serial.println(ADCSum);*/
        //Serial.print("voltage=");
        //Serial.println(voltage/10000);
        ADCSum = 0;
        value = map(voltage, 1000, 17500, -40, 125);
        //Serial.print("value=");
        //Serial.println(value);
      }
  }
  //Serial.println(value);
  return value;
}

void ohp()
{
  digitalWrite(EXT_PWR_DWN, HIGH);
  display.clearDisplay();
  display.cp437(true);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setCursor(6, 1);
  display.print("WARNING!!!");
  display.setTextSize(1);
  display.setCursor(46, 27);
  display.print("System");
  display.setCursor(33, 41);
  display.print("Overheating");
  display.display();
  while(1);
}

void selectClockSrcPath(uint8_t path)
{
  switch (path)
  {
    case CLOCK_SOURCE_TCXO_INDEX:
      digitalWrite(TCXO_POWER_PIN, HIGH);
      digitalWrite(TCXO_PATH_PIN, HIGH);
      digitalWrite(EXTERANL_SRC_PATH_PIN, LOW);
    break;
    case CLOCK_SOURCE_EXT_TCXO_INDEX:  //////////// ЭТО ЭКСТЕРНАЛ TCXO!!!!!!!!!!  //
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;
    case CLOCK_SOURCE_EXT_OSC_INDEX:
      digitalWrite(TCXO_POWER_PIN, LOW);
      digitalWrite(TCXO_PATH_PIN, LOW);
      digitalWrite(EXTERANL_SRC_PATH_PIN, HIGH);
    break;    
  }
}

float getInputVoltage()
{
  uint16_t voltage=0; //mV
  analogReference(DEFAULT); 
  delay(10);
  voltage = map(analogRead(INPUT_VOLTAGE_PIN), 0, 1023, 0, ANALOG_REFERENCE_VOLTAGE); //4.95
  return (float(voltage) / 1000.0 / VOLTAGE_DIVIDER_FACTOR) + 1.0;
}

void displayPowerWarning()
{
  static const unsigned char PROGMEM image_alert_bicubic_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x0f,0xf0,0x00,0x00,0x00,0x00,0x0e,0x70,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x3c,0x3c,0x00,0x00,0x00,0x00,0x38,0x1c,0x00,0x00,0x00,0x00,0x78,0x1e,0x00,0x00,0x00,0x00,0x70,0x0e,0x00,0x00,0x00,0x00,0xf0,0x0f,0x00,0x00,0x00,0x00,0xe3,0xc7,0x00,0x00,0x00,0x01,0xc3,0xc3,0x80,0x00,0x00,0x03,0xc3,0xc3,0xc0,0x00,0x00,0x03,0x83,0xc1,0xc0,0x00,0x00,0x07,0x83,0xc1,0xe0,0x00,0x00,0x07,0x03,0xc0,0xe0,0x00,0x00,0x0e,0x03,0xc0,0xf0,0x00,0x00,0x0e,0x03,0xc0,0x70,0x00,0x00,0x1c,0x03,0xc0,0x38,0x00,0x00,0x3c,0x03,0xc0,0x3c,0x00,0x00,0x38,0x03,0xc0,0x1c,0x00,0x00,0x78,0x03,0xc0,0x1e,0x00,0x00,0x70,0x03,0xc0,0x0e,0x00,0x00,0xe0,0x03,0xc0,0x07,0x00,0x01,0xe0,0x03,0xc0,0x07,0x80,0x01,0xc0,0x03,0xc0,0x03,0x80,0x03,0xc0,0x01,0x80,0x03,0xc0,0x03,0x80,0x00,0x00,0x01,0xc0,0x07,0x80,0x00,0x00,0x01,0xe0,0x07,0x00,0x01,0x80,0x00,0xe0,0x0e,0x00,0x03,0xc0,0x00,0x70,0x1e,0x00,0x03,0xc0,0x00,0x78,0x1c,0x00,0x01,0x80,0x00,0x38,0x3c,0x00,0x00,0x00,0x00,0x3c,0x38,0x00,0x00,0x00,0x00,0x1c,0x38,0x00,0x00,0x00,0x00,0x1c,0x3f,0xff,0xff,0xff,0xff,0xfc,0x1f,0xff,0xff,0xff,0xff,0xf8,0x01,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  display.clearDisplay();
  display.setTextColor(1);
  display.setTextSize(2);
  display.setCursor(23, 1);
  display.print(F("WARNING"));
  display.setTextWrap(false);
  display.setCursor(49, 17);
  display.print(F("12 VDC"));
  display.setTextSize(1);
  display.setCursor(49, 35);
  display.print(F("POWER SUPPLY"));
  display.setTextSize(2);
  display.setCursor(62, 45);
  display.print("ONLY");
  display.drawBitmap(1, 11, image_alert_bicubic_bits, 48, 48, 1);
  display.setTextSize(1);
  display.display();
}

bool isPLLLocked()
{
  static uint32_t lastTimeReading = 0;
  static bool PLLLocked = false;

  if ((millis() - lastTimeReading) < 1000) return PLLLocked;
  lastTimeReading = millis();

  PLLLocked = bitRead(DDS.readRegister4Bytes(USR0), PLL_LOCK);

  return PLLLocked;
}
