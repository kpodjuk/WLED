#pragma once

#include "wled.h"

uint16_t presetWhenMovementDetected = 1;
uint16_t presetWhenNoMovementDetected = 2;

class UsermodPirSensor : public Usermod
{
private:
// constants
    const uint8_t PIRsensorPin = 13; // D7 on hardware
    uint32_t m_switchOffDelay = 60000; // 1 min delay before switch off after the sensor state goes LOW

public:
  void setup()
  {
    // Serial.println("Hello from usermod pir sensor!");
    pinMode(PIRsensorPin, INPUT);
  }

  void connected()
  {
    Serial.println("WIFI CONNECTED!");
  }

  void loop()
  {
    static int counter = 0;
    
    if(digitalRead(PIRsensorPin) == HIGH){
      counter++;
      Serial.printf("movement detected %i!!!\n", counter);
      applyPreset(presetWhenMovementDetected);
    } else {
      // applyPreset(presetWhenNoMovementDetected); 

    }
  }
};