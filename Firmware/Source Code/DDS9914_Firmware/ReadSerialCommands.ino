

const char HELP_STRING [] PROGMEM = 
          "F - Set Frequency in Hz (100000 - DDS_Core_Clock/2)\n"
          "P - Set Output Power in dBm (-68 - 0)\n"
          "M - Get Model\n"
          "V - Get Firmware Version\n"
          "h - This Help\n"
          "; - Commands Separator"
          "\n"
          "Example:\n"
          "F100000;P-2\n"
          "Set Frequency to 100 kHz, and Output Power to -2 dBm.\n"
          "Any number of commands in any order is allowed.";

void ReadSerialCommands()
{
  int32_t value=0;
  char command;

  if (serialbuffer.available())
  {
    GParser data(serialbuffer.buf, ';');
    int commandsCounter = data.split();

    for (int i=0; i < commandsCounter; i++)
    {
      sscanf(data[i], "%c%ld", &command, &value);
      switch (command)
      {
        case 'F': //RF Frequency
          if (inRange(value, LOW_FREQ_LIMIT, ui32HIGH_FREQ_LIMIT))
          {
            Serial.print(F("Set freq.: "));
            Serial.println(value);
            ui32CurrentOutputFreq = value;
            HZ.value = value % 1000;
            value = value / 1000;
            KHZ.value = value % 1000;
            value = value / 1000;
            MHZ.value = value % 1000;
            value = value / 1000;
            GHZ.value = value;
          } else Serial.println("Frequency is OUT OF RANGE (" + String(LOW_FREQ_LIMIT) + " - " + String(ui32HIGH_FREQ_LIMIT) + ")");
        break;

        case 'P': //Power, dBm
          if (inRange(value, -1*Amplitude.maxValue, Amplitude.minValue))
          {
            Serial.print(F("Set Power.: "));
            Serial.println(value);
            Amplitude.value = -1 * value;
          } else Serial.println("Power is OUT OF RANGE (-" + String(Amplitude.maxValue) + " - " + String(Amplitude.minValue) + ")");
        break;

        case 'V': //Firmware Version request
          Serial.println(FIRMWAREVERSION);
          //Serial.println(value);
        break;

        case 'M': //Model request
          Serial.println(F("DDS9914/5 v2.2"));
          //Serial.println(value);
        break;

        case 'h': //Model request
          Serial.println((const __FlashStringHelper *) HELP_STRING);
        break;

        default:
        Serial.print(F("Unknown command:"));
        Serial.println(command);
        Serial.println((const __FlashStringHelper *) HELP_STRING);
      } //switch
    } //for

    //ui32CurrentOutputFreq=GetFreq(); 
    DDS.setFreq(ui32CurrentOutputFreq, 0); 
    DDS.setAmpdB(Amplitude.value *-1, 0);
  }
}

bool inRange(int32_t val, int32_t minimum, int32_t maximum)
{
  return ((minimum <= val) && (val <= maximum));
}