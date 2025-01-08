#include "DisplayMenu.h"

void DisplayMenu(uint8_t menuType) //menuType {MAIN_MENU=0, CORE_CLOCK_MENU=1}
{
  switch (menuType)
  {
    case MAIN_MENU:
      display.clearDisplay();
      display.setTextSize(2);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0, 0);     // Start at top-left corner
      //display.cp437(true);         // Use full 256 char 'Code Page 437' font

      display.println(F("DDS AD9914"));

      display.setTextSize(1);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0, 16);     // Start at top-left corner

      display.print(F("Freq., Hz:"));
      if (isPWR_DWN) display.print(F(" OFF"));
        //else display.print(F("    "));

      display.setCursor(103, 16);
      //display.print(F("100"));
      display.print(PreZero(temperature, 2));
      display.setCursor(116, 14);
      //display.print("o");
      display.drawPixel(118, 16, 1);
      display.drawPixel(119, 16, 1);
      display.drawPixel(117, 17, 1);
      display.drawPixel(117, 18, 1);
      display.drawPixel(118, 19, 1);
      display.drawPixel(119, 19, 1);
      display.drawPixel(120, 18, 1);
      display.drawPixel(120, 17, 1);
      display.setCursor(122, 17);
      display.print(F("C"));

      display.drawPixel(11, 39, WHITE); //
      display.drawPixel(11, 40, WHITE); //
      display.drawPixel(12, 39, WHITE); // dot
      display.drawPixel(12, 40, WHITE); //

      display.setTextSize(2);      // Normal 1:1 pixel scale
      if (curItem == &GHZ) {if (GHZ.bBlink==true) display.setTextColor(GHZ.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        else display.setTextColor(WHITE);
      display.setCursor(1, 26);
      display.print(GHZ.value);

      display.drawPixel(47, 39, WHITE); //
      display.drawPixel(47, 40, WHITE); //
      display.drawPixel(48, 39, WHITE); // dot
      display.drawPixel(48, 40, WHITE); //

      if (curItem == &MHZ) {if (MHZ.bBlink==true) display.setTextColor(MHZ.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        else display.setTextColor(WHITE);
      display.print(PreZero(MHZ.value));

      display.drawPixel(83, 39, WHITE); //
      display.drawPixel(83, 40, WHITE); //
      display.drawPixel(84, 39, WHITE); // dot
      display.drawPixel(84, 40, WHITE); //

      if (curItem == &KHZ) {if (KHZ.bBlink==true) display.setTextColor(KHZ.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        else display.setTextColor(WHITE);
      display.print(PreZero(KHZ.value));

      if (curItem == &HZ) {if (HZ.bBlink==true) display.setTextColor(HZ.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      else display.setTextColor(WHITE);
      display.print(PreZero(HZ.value));

      display.setTextSize(1);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0, 42);     // Start at top-left corner

      display.print(F("Amplitude:"));

      switch (FreqInRange())
      {
        case 1: display.print(stTooHighFreq.GetText()); break;
        case -1: display.print(stTooLowFreq.GetText()); break;
      }
    
      //display.print("  Mod:"); //NONE");
      //if (curItem==&) display.setTextColor(BLACK, WHITE);
      // else display.setTextColor(WHITE);
      //display.println(ModName[ModIndex]);

      //display.drawLine(68,42, 68,64, WHITE);

      display.setTextSize(2);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(1, 50);     // Start at top-left corner

      //if (Amplitude.value < 0) display.print(F("+")); //reverse sign
      if (Amplitude.value > 0) display.print(F("-")); //reverse sign
      if (Amplitude.value == 0) display.print(F(" "));
      if (curItem == &Amplitude) {if (Amplitude.bBlink==true) display.setTextColor(Amplitude.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      else display.setTextColor(WHITE);
      display.print(PreZero(abs(Amplitude.value), 2));

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(37, 56);
      display.println(F("dBm"));

      display.setTextSize(1);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE);
      display.setCursor(72, 56);     // Start at top-left corner

      display.println(F("GRA&AFCH"));
      display.display();
      break; //************ MAIN MENU END ***************

    // ************ SETUP MENU ****************
    case CORE_CLOCK_MENU:
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(40, 0);
      display.cp437(true);
      display.println(F("SETUP"));

      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 17);
      display.print(F("Clock Src"));
      display.setCursor(52, 18);
      display.print(F(":"));
      if (curItem == &ClockSrc) {if (ClockSrc.bBlink==true) display.setTextColor(ClockSrc.GetColor(), WHITE); 
        else display.setTextColor(BLACK, WHITE);}
      if (ClockSrc.ClockSourceNames[ClockSrc.value].length() < 12) display.setCursor(58, 17);
        else display.setCursor(56, 17);
      display.print(ClockSrc.ClockSourceNames[ClockSrc.value]);

      display.setTextColor(WHITE);
      display.setCursor(0, 26);
      display.print(F("Clock Freq"));
      display.setCursor(58, 27);
      display.print(F(":"));
      //display.print(" ");
      if ((ClockSrc.value == TCXO_OCXO) || (ClockSrc.value == EXT_TCXO_OCXO))
      {
        if (curItem == &ClockFreq) {if (ClockFreq.bBlink==true) display.setTextColor(ClockFreq.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        display.print(ClockFreq.Ref_Clk[ClockFreq.value] / 1000000);
      } else
      {
        if (curItem == &ExtClockFreqGHz) {if (ExtClockFreqGHz.bBlink==true) display.setTextColor(ExtClockFreqGHz.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        display.print(ExtClockFreqGHz.value);
        display.setTextColor(WHITE);
        if (curItem == &ExtClockFreqMHz) {if (ExtClockFreqMHz.bBlink==true) display.setTextColor(ExtClockFreqMHz.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
        display.print(PreZero(ExtClockFreqMHz.value));
      }
      display.setTextColor(WHITE);
      display.print(F(" MHz"));

      //*********
      display.setTextSize(1);
      display.setCursor(0, 35);
      display.print(F("Core Clock"));
//      display.print("Clock:");
      display.setCursor(57, 35);
      display.print(F(":"));
      display.setCursor(64, 35);
      display.setTextColor(WHITE);
      //display.print(" ");
      if (curItem == &DDSCoreClock) {if (DDSCoreClock.bBlink==true) display.setTextColor(DDSCoreClock.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      //display.print(DDSCoreClock.DDSCoreClock / 1000000);
      display.print((uint32_t)(DDSCoreClock.GetDDSCoreClock() / 1000000));
      display.setTextColor(WHITE);
      display.print(F(" MHz"));
      //***********

      display.setCursor(0, 55);
      if (curItem == &CoreClockSave) display.setTextColor(BLACK, WHITE);
      display.println(F("SAVE"));

      display.setTextColor(WHITE);
      display.setCursor(103, 55);
      if (curItem == &CoreClockExit) display.setTextColor(BLACK, WHITE);
      display.println(F("EXIT"));

      /*display.setTextColor(WHITE);
      display.setCursor(30, 55);
      if (SetupMenuPos == CLOCK_MENU_CORE_CLOCK_OFFSET) display.setTextColor(BLACK, WHITE);
      display.print("Offset");
      display.setTextColor(WHITE);
      if (ClockOffset >= 0) display.print(" ");
      else display.print("-");
      display.print(abs(ClockOffset));
      */
      display.display();
      
      break;
  }
}

void DisplayHello()
{
  display.clearDisplay();
  display.cp437(true);
  display.setTextSize(2);
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);
  display.print(F("Hint:"));
  display.setTextSize(1);
  display.setCursor(62, 0);
  display.print(F("Firmware"));
  display.setCursor(62, 8);
  display.print(F("ver.: "));
  display.print(FIRMWAREVERSION);
  display.setCursor(0, 16);

  display.setTextSize(2);
  display.print(F("Hold "));
  //display.setTextColor(BLACK, WHITE);
  display.print(F("MODE"));
  display.setTextColor(WHITE);
  display.println(F(" to enter"));
  display.println(F("setup"));

  display.display();
}

String PreZero(int Digit, uint8_t Qty=3)
{
  switch (Qty)
  {
    case 3:
      if ((Digit < 100) && (Digit >= 10)) return "0" + String(Digit);
      if (Digit < 10) return "00" + String(Digit);
      return String(Digit);
    case 2:
      if (Digit < 10) return "0" + String(Digit);
      return String(Digit);
  }
}
