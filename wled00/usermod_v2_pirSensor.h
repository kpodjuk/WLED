#pragma once

#include "wled.h"

uint16_t presetWhenMovementDetected = 1; // labeled as "Save to ID" in webinterface
uint16_t presetWhenNoMovementDetected = 2;

class UsermodPirSensor : public Usermod
{
private:
  uint32_t previousCheckTime;
  uint32_t currentCheckTime;
  bool previousPirSensorState;
  bool currentPirSensorState;

  // constants
  const uint8_t PirSensorPin = 13;         // D7 on hardware
  const uint32_t m_switchOffDelay = 60000; // 1 min delay before switch off after the sensor state goes LOW
  const uint16_t checkFrequencyMs = 1000;  // how often to check sensor in milliseconds

public:
  UsermodPirSensor() : previousCheckTime(0),
                       currentCheckTime(0),
                       previousPirSensorState(0),
                       currentPirSensorState(0)
  {
  }
  void setup()
  {
    // Serial.println("Hello from usermod pir sensor!");
    pinMode(PirSensorPin, INPUT);
  }

  void connected()
  {
    Serial.println("WIFI CONNECTED!");
  }

  void loop()
  {
    currentCheckTime = millis();
    if (currentCheckTime - previousCheckTime > checkFrequencyMs)
    {
      previousCheckTime = currentCheckTime;
      // Serial.println("Checking sensor!");
      checkPirSensor();
    }
  }

  // check pir sensor state and apply preset
  void checkPirSensor()
  {
    currentPirSensorState = digitalRead(PirSensorPin);
    if (currentPirSensorState != previousPirSensorState)
    {
      // Serial.println("Makes sense to apply preset!");
      // only set preset when it needed == when pirSensorState changes
      if (currentPirSensorState)
      {
        // Serial.println("presetWhen MovementDetected");
        applyPreset(presetWhenMovementDetected);
      }
      else
      {
        // Serial.println("presetWhen NoMovementDetected");
        applyPreset(presetWhenNoMovementDetected);
      }
    }

    previousPirSensorState = currentPirSensorState;
  }

  void applyCorrectPreset();
};
