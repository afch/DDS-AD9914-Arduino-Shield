/* 
   AD9914.h - AD9914 DDS communication library
   Created by Ben Reschovsky, 2016.
   JQI - Strontium - UMD
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   aunsigned long with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AD9914_h
#define AD9914_h

#include "Arduino.h"

//Define pin mappings:

#define CSPIN 6                 // DDS chip select pin. Digital input (active low). Bringing this pin low enables detection of serial clock edges.
#define OSKPIN 47 //not actually connected               // DDS Output Shift Keying. Digital input.
#define PS0PIN 7               // DDS PROFILE[0] pin. Profile Select Pins. Digital input. Use these pins to select one of eight profiles for the DDS.
#define PS1PIN 8               // DDS PROFILE[1] pin. Profile Select Pins. Digital input. Use these pins to select one of eight profiles for the DDS.
#define PS2PIN 9               // DDS PROFILE[2] pin. Profile Select Pins. Digital input. Use these pins to select one of eight profiles for the DDS.
#define IO_UPDATEPIN  16         // DDS I/O_UPDATE pin. Digital input. A high on this pin transfers the contents of the buffers to the internal registers.
#define RESETPIN 17      // DDS MASTER_RESET pin. Digital input. Clears all memory elements and sets registers to default values.

#define SYNCIO_PIN 5
#define DROVER A2
#define TEMP A3
#define DRHOLD 48
#define DRCTL 49

#define F0 10
#define F1 11
#define F2 12
#define F3 13

#define EXT_PWR_DWN 46

#define BACKPIN 3
#define MODE 2
#define A 18
#define B 19

#define CLOCK_SOURCE_TCXO_INDEX 0
#define CLOCK_SOURCE_EXT_TCXO_INDEX 1
#define CLOCK_SOURCE_EXT_OSC_INDEX 2
#define EXTERANL_SRC_PATH_PIN A1 //switcher V2, выход на коммутатор источников тактирования
#define TCXO_PATH_PIN A0 //switcher V1, выход на коммутатор источников тактирования
#define TCXO_POWER_PIN A5 //выход на светодиод, который должен включаться, если настроено внешнее тактирование. Зажигается 0
#define PLL_LOCK_LED_PIN A6 //выход на светодиод, который должен включаться, если в соответсвующем регистре DDS выставлен бит обозначающий что PLL защелкнулся. Зажигается 1
#define INPUT_VOLTAGE_PIN A7 //ВХОД (VIN_MEAS)на который подадется напряжение питания (через делитель), если напряжение ниже или выше пороговых значений, то DDS должна отключатся! (пусть пороговыми будут значения 11 и 13 вольт). Отключение через управление ножкой EXT_PWR_DWN
#define VOLTAGE_DIVIDER_FACTOR 0.09091 // 1k/(1k+10k)=0.0909
#define ANALOG_REFERENCE_VOLTAGE 4950

#define MAX_PARAM_NUM 11

/***Register Name********************/
#define CFR1 0x00
/***Bit Name********************/  
#define EXTERNAL_POWER_DOWN_CONTROL 0x08
#define SDIO_INPUT_ONLY             0x02
#define OSK_ENABLE                  0x01
#define VCO_CAL_ENABLE              0x01
#define ENABLE_SINE_OUTPUT          0x01
/*********************************/

/***Register Name********************/
#define CFR2                    0x01
/***Bit Name********************/ 
#define SYNC_CLK_ENABLE         0x08
#define SYNC_OUT_IN_MUX_ENABLE  0x01 // always ON for AD9914
#define PROFILE_MODE_ENABLE     0x80
/**************/

/***Register Name********************/
#define CFR3          0x02
/***Bit Name [7:0]********************/ 
#define Manual_ICP_selection  0x40
#define ICP_125uA             0x00
#define ICP_250uA             0x08
#define ICP_375uA             0x10
#define ICP_500uA             0x18
#define ICP_625uA             0x20
#define ICP_750uA             0x28
#define ICP_875uA             0x30
#define ICP_1000uA            0x38
#define Lock_detect_enable    0x04
#define REF_CLK_cycles_128    0x00
#define REF_CLK_cycles_256    0x01
#define REF_CLK_cycles_512    0x02
#define REF_CLK_cycles_1024   0x03
/***Bit Name [8:15]********************/ 
/*The N divider value one part of the total PLL multiplication available. The second part
is the fixed divide by two element in the feedback path. Therefore, the total PLL multiplication value
is 2N. The valid N divider range is 10× to 255×. The default N value for Bits[15:8] = 25. This sets the
total default PLL multiplication to 50× or 2N.*/
//Feedback_Divider_N 
/***Bit Name [23:16]********************/
#define Input_divider_reset  0x40
#define Input_divider_1      0x00
#define Input_divider_2      0x10
#define Input_divider_4      0x20
#define Input_divider_4      0x30
#define Doubler_enable       0x08
#define PLL_enable           0x04
#define PLL_input_divider_enable 0x02
#define Doubler_clock_edge   0x01
/***Bit Name [31:24]********************/
// Write 0x00




/***Register Name********************/
#define CFR4                   0x03
/***Bit Name********************/ 
#define DAC_CAL_ENABLE         0x01 
#define Aux_divider_power_down 0x04  
/**********************************/



#define PROFILE0_REGISTER 0x0B
#define PROFILE0_PHASE_AMPLITUDE 0x0C

/***Register Name********************/
#define USR0                  0x1B
/***Bit Name********************/ 
#define PLL_LOCK              0x18 
/**********************************/

class AD9914
{
    public: 
        // Constructor function. 
        AD9914(byte ssPin, byte resetPin, byte updatePin, byte ps0, byte ps1, byte ps2, byte osk);

        // Initialize with refClk frequency
        void initialize(unsigned long, uint8_t);

        // Initialize with refIn frequency, and clock multiplier value
        //void initialize(unsigned long, byte);

        // Reset the DDS
        void reset();

        // Update to load newly written settings
        void update();
        
        // Gets current frequency
        unsigned long getFreq(byte);
        unsigned long getFreq();
        
        // Sets frequency
        void setFreq(unsigned long, byte);
        // Sets frequency
        void setFreq(unsigned long); //default to profile 0

        // Gets current frequency tuning word
        unsigned long getFTW(byte);
        unsigned long getFTW();
        
        // Sets frequency tuning word
        void setFTW(unsigned long, byte);
        void setFTW(unsigned long);
        
        //write scaled amplitude for the selected profile
        void setAmp(double scaledAmp, byte profile);
        void setAmp(double scaledAmp);
        void setAmpdB(double scaledAmpdB, byte profile);
        void setAmpdB(double scaledAmpdB);
        
        // Gets current amplitude
        double getAmp(byte);
        double getAmp();
        
        // Gets current amplitude in dB
        double getAmpdB(byte);
        double getAmpdB();
        
        // Gets current amplitude tuning word
        unsigned long getASF(byte);
        unsigned long getASF();
        

        // places DDS in linear sweep mode
        //void linearSweep(unsigned long, unsigned long, unsigned long, byte, unsigned long, byte);

        //enable profile mode
        void enableProfileMode();

        //disable profile mode
        void disableProfileMode();
        
        //enable OSK
        void enableOSK();
        
        //disable OSK
        void disableOSK();
        
        //Get profile mode status
        boolean getProfileSelectMode();
        
        //Get OSK mode status
        boolean getOSKMode();

        //enable the Sync Clck output
        void enableSyncClck();

        //disable the Sync Clck output
        void disableSyncClck();
        
        //Change active profile mode:
        void selectProfile(byte);
        
        //Get currently active profile
        byte getProfile();
        
        uint32_t readRegister4Bytes(byte registerAddress);
        void writeRegister(byte[2], byte[1024]);
        


    private:
        // Instance variables that hold pinout mapping
        // from arduino to DDS pins.
        byte _ssPin, _resetPin, _updatePin, _ps0, _ps1, _ps2, _osk;

        // Instance variables for frequency _freq, frequency tuning word _ftw,
        // reference clock frequency _refClk, etc.
        unsigned long _freq[8], _ftw[8], _refClk, _refIn, _asf[8];
        double _scaledAmp[8], _scaledAmpdB[8];
        
        byte _activeProfile;
        
        // Instance variables to keep track of the DDS mode:
        boolean _profileModeOn, _OSKon;


        // function to write data to register. 
        //void writeRegister(byte[2], byte[1024]);
        
        // function to toggle the DAC CAL register bit and calibrate the DAC
        void dacCalibrate();
        
        // write amplitude tuning word to device
        void writeAmp(long ampScaleFactor, byte profile);

        // DDS frequency resolution
         double RESOLUTION;// = 4294967296; // sets resolution to 2^32 = 32 bits. Using type double to avoid confusion with integer division...

};

#endif
