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

      display.print(F("Frequency, [Hz]:"));
      if (isPWR_DWN) display.print(F(" OFF"));
        //else display.print(F("    "));

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

      display.print("-");
      if (curItem == &Amplitude) {if (Amplitude.bBlink==true) display.setTextColor(Amplitude.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      else display.setTextColor(WHITE);
      display.print(PreZero(abs(Amplitude.value), 2));

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(37, 56);
      display.println("dBm");

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
      display.println("SETUP");

      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setCursor(0, 17);
      display.print("Clock Src: ");
      if (curItem == &ClockSrc) {if (ClockSrc.bBlink==true) display.setTextColor(ClockSrc.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      display.print(ClockSrc.ClockSourceNames[ClockSrc.value]);

      display.setTextColor(WHITE);
      display.setCursor(0, 26);
      display.print("Clock Freq:");
      display.print(" ");
      if (ClockSrc.value==TCXO_OCXO)
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
      display.print(" MHz");

      //*********
      display.setTextSize(1);
      display.setCursor(0, 35);
      display.print("Core Clock");
//      display.print("Clock:");
      display.setTextColor(WHITE);
      display.print(" ");
      if (curItem == &DDSCoreClock) {if (DDSCoreClock.bBlink==true) display.setTextColor(DDSCoreClock.GetColor(), WHITE); else display.setTextColor(BLACK, WHITE);}
      //display.print(DDSCoreClock.DDSCoreClock / 1000000);
      display.print((uint32_t)(DDSCoreClock.GetDDSCoreClock() / 1000000));
      display.setTextColor(WHITE);
      display.print(" MHz");
      //***********

      display.setCursor(0, 55);
      if (curItem == &CoreClockSave) display.setTextColor(BLACK, WHITE);
      display.println("SAVE");

      display.setTextColor(WHITE);
      display.setCursor(103, 55);
      if (curItem == &CoreClockExit) display.setTextColor(BLACK, WHITE);
      display.println("EXIT");

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
  display.print("Hint:");
  display.setTextSize(1);
  display.setCursor(62, 0);
  display.print("Firmware");
  display.setCursor(62, 8);
  display.print("ver.: ");
  display.print(FIRMWAREVERSION);
  display.setCursor(0, 16);

  display.setTextSize(2);
  display.print("Hold ");
  //display.setTextColor(BLACK, WHITE);
  display.print("MODE");
  display.setTextColor(WHITE);
  display.println(" to enter");
  display.println("setup");

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
