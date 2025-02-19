# DDS AD9914 / AD9915 Arduino Shield
DDS (Direct Digital Synthesis) Analog Devices AD9914/AD9915 Arduino Shield by GRA &amp; AFCH

DDS (Direct Digital Synthesis) Analog Devices AD9915 Arduino Shield by GRA &amp; AFCH <br/>
The difference between the two RF Shield Units is only in the maximum clocking frequency of the DDS core <br/>
In DDS AD9915 Shield Unit it is nominally 2.5 GHz and it can be overclocked to 3 GHz <br/>
In DDS AD9914 Shield Unit it is nominally higher than 3.5 GHz and can be overclocked to 4 GHz <br/>
The firmware is the same for AD9914 and AD9915 except for the inscription on the OLED screen, you need to change line 14 in the DisplayMenu.ino file to display.printin(F("DDS AD9914")); <br/>

Web-site: https://gra-afch.com
Direct link to category:  https://gra-afch.com/product-category/rf-units/

# What's New in Version 3
* A software-controlled clock source switch has been added.
* Voltage monitoring of the power supply has been added (acceptable range: 10 to 14 volts).
* An "Output on/off" button has been added to enable and disable the output signal.
* A LED indicator for the PLL lock has been added.
* LED indicators of active SMA inputs/outputs

# GA_Flasher can be used to upload compiled firmware from "Firmware Compiled (.HEX File)" folder: https://github.com/afch/GA_Flasher

"Firmware Compiled (.HEX File)" - folder contains pre-Compiled firmware (.HEX File), that can be uploaded via "AvrDude", GA_Flasher or any other software for flashing Atmega 2560

"Firmware Source (.INO File)" - folder contains source of firmware that can be compiled (modified if necessary) and uploaded via Arduino IDE: https://www.arduino.cc/en/Main/Software

"Libraries" - contains libraries that are only needed for compilation: place them to "C:\Users\[USER]\Documents\Arduino\libraries", where [USER] - your windows username.

This AD9959 Shield can be easily connected to Arduino Mega without additional wires and converters. All functions of the DDS AD9914/AD9915 are brought to the contacts of the Arduino Mega thanks to this you can fully reveal all the capabilities of the DDS AD9914/AD9915.

Key Benefits:

* Low harmonics no more than -60dB. An output RF transformer is used for the correct operation of the current mirror.
* balancing transformer for TCXO and REF CLK IN options
* Small spur
* 4 layer board. Signal lines TOP and Bottom, inner layers Ground and Power.
* Low Noise LDO Stabilizers
* Separate power supply for all analog and digital lines (1.8 and 3.3V), 5pcs IC voltage stabilizers are used. Additionally, there is an RF Ferrite bead interchange.
* High-speed decoupling Level converter and TTL 5V matching
* one of these types of clock source can be used/installed:

1. Onboard TCXO/OCXO - 5 - 50 MHz 1ppm (PLL),
2. External TCXO/OCXO - 5 - 50 MHz (PLL)
3. External oscillator - 500 MHz to 4000 MHz (REF CLK IN)

* Easy to connect OLED display.
* Convenient and fast control with an encoder
* The synthesizer is capable to generate sine wave.
* The software allows you to select and configure the frequency of the clock generator through the user menu (without the need to recompile the program).
* Any settings can be stored in non-volatile EEPROM memory (located at Arduino Mega).
* Basic settings are applied and saved automatically.
* This shield support overclocking the AD9914/AD9915 core up to 4000 MHz.
* DDS AD9914/AD9915 Shield has ability to generate a signal up to 1999 MHz.

# The switching of clock sources:
The switching of clock sources is performed through the clock menu and is handled by an onboard software-controlled switch. It does not require the installation or removal of any components on the board to switch between clock sources: TCXO/OCXO, External TCXO/OCXO or External Oscillator.

# List of Serial Port Commands:
Starting with version 0.82, the ability to control via the serial port has been added.

  F - Set Frequency in Hz (100000 - CoreFrequency/2)  
  P - Set Output Power in dBm (-68 - 0)  
  M - Get Model  
  V - Get Firmware Version  
  E - Enable Output  
  D - Disable Output  
  h - Help  
  ; - Commands Separator  
          
Example:  
  F100000;P-2  
Set Frequency to 100 kHz, and Output Power to -2 dBm.  
Any number of commands in any order is allowed.  

# Serial Port Settings:

  Speed - 115200 Bouds  
  Data Bits - 8  
  Stop Bits - 1  
  Parity - No  
  DTR - OFF  
# Windows:

An example of setting up a serial port in the Windows console:
  <pre>
  mode COM3 baud=115200 DTR=OFF Data=8
  </pre>
  
Usage example:
  <pre>
  echo F100000000 > COM3
  </pre>
# Ubuntu 22.04:

An example of setting up a serial port in the Ubuntu:
  <pre>
  sudo usermod -aG dialout $USER_NAME$
  sudo chmod a+rw /dev/ttyUSB0
  sudo stty -F /dev/ttyUSB0 115200 cs8 ixoff -hupcl -echo
  </pre>
  
Usage example:
  <pre>
  echo "F100000000" > /dev/ttyUSB0
  </pre>
