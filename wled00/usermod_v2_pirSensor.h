#pragma once

#include "wled.h"

// #define DEBUG_PIR_SENSOR

// disables pir sensor whole functionality
// #define DISABLE_PIR_SENSOR_MONITORING
bool motionSensingGlobal = false; //global var with information if motion sensing should be active

class UsermodPirSensor : public Usermod
{
private:
  // *********** PRIVATE VARIABLES ***********
  uint32_t previousCheckTime;
  uint32_t currentCheckTime;
  bool currentPinState;
  bool keepMovementFlag;
  bool checkAnywaysFlag;
  uint32_t keepMovementCounter; // counter to keep HIGH/MOVED status for a x time after movement was detected
  uint16_t currentPreset;
  uint16_t previousPreset;

  // *********** PARAMS ***********
  const uint8_t pirSensorPin = 16; // D0 on hardware
  const float keepMovementDelayMinutes = 5; // x min delay before switch off after the sensor state goes LOW
  const uint16_t presetWhenMovementDetected = 1; // labeled as "Save to ID" in webinterface, 0 = default
  const uint16_t presetWhenNoMovementDetected = 2; 

  // *********** OTHER CONSTS ***********
  const uint32_t keepMovementDelaySeconds = (uint32_t)(keepMovementDelayMinutes * (float)60); 
  const uint16_t checkFrequencyMs = 1000; // how often to check sensor in milliseconds, if you want "keepMovementDelaySeconds" to make sense this has to stay at 1000ms


// debug var
#ifdef DEBUG_PIR_SENSOR
  int resultingPreset;
#endif

public:
  UsermodPirSensor() : previousCheckTime(0),
                       currentCheckTime(0),
                       currentPinState(LOW),
                       keepMovementFlag(false),
                       checkAnywaysFlag(false),
                       keepMovementCounter(0),
                       currentPreset(0),
                       previousPreset(0)
  {
  }
  void setup()
  {
    pinMode(pirSensorPin, INPUT);
  }

  void connected()
  {
#ifdef DEBUG_PIR_SENSOR
    Serial.println("WIFI CONNECTED!");
#endif
  }

  void loop()
  {
    if (motionSensingGlobal)
    {
      // Serial.println("motionSensingGlobal=true");
      checkPirSensorPeriodically();
    }
  }

  // check pir sensor state and apply preset
  void checkPirSensorPeriodically()
  {

    currentCheckTime = millis();
    if (currentCheckTime - previousCheckTime > checkFrequencyMs)
    {
      previousCheckTime = currentCheckTime;
      checkSensorState();
    }
    else
    {
      // no need to check, not enough time passed
    }
  }

  void checkSensorState()
  {
    // Serial.println("checkSensorState()");

    currentPinState = digitalRead(pirSensorPin);

    if (checkAnywaysFlag && keepMovementFlag == false)
    {
      checkAnywaysFlag = false;
      // change on pin detected, and i'm not told to keep state
      if (currentPinState == HIGH)
      {
        keepMovementFlag = true; // set flag to wait with going back to non movement state
        keepMovementCounter = 0; // reset counter
        applyPresetIfNeeded(presetWhenMovementDetected);
#ifdef DEBUG_PIR_SENSOR
        resultingPreset = 1;
#endif
      }
      else if (currentPinState == LOW)
      {
        applyPresetIfNeeded(presetWhenNoMovementDetected);
#ifdef DEBUG_PIR_SENSOR
        resultingPreset = 0;
#endif
      }
    }

    // keep restarting the counter if pin state is kept HIGH, so 30sec without movement -> no movement preset
    if (currentPinState == HIGH)
      keepMovementCounter = 0;

    // do the counting for keeping movement state
    if (keepMovementCounter < keepMovementDelaySeconds && keepMovementFlag)
    {
      keepMovementCounter++;
    }
    else
    {
      // time passed, can turn off flag now
      keepMovementFlag = false;
      keepMovementCounter = 0;
      // special flag to make it check pin even with no change detected
      checkAnywaysFlag = true;
    }
    // Debug print
#ifdef DEBUG_PIR_SENSOR
    Serial.printf("pinState=%i\tkeepMovementCounter=%i\tkeepMovementFlag=%i\toutput(0-no mov)=%i\n", currentPinState, keepMovementCounter, keepMovementFlag, resultingPreset);
#endif
  }

  void applyPresetIfNeeded(uint16_t preset)
  {

    if (preset != previousPreset)
    {
      // Serial.printf("!!!!!! ACTUALLY CHANGED PRESET NOW, PRESET CHANGED TO:%i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n", currentPreset);
      // Serial.printf("apply preset=%i\n", preset);
      applyPreset(preset);
    }
    previousPreset = preset;
  }

  /*
   * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
   * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
   * If you want to force saving the current state, use serializeConfig() in your loop().
   *
   * CAUTION: serializeConfig() will initiate a filesystem write operation.
   * It might cause the LEDs to stutter and will cause flash wear if called too often.
   * Use it sparingly and always in the loop, never in network callbacks!
   *
   * addToConfig() will also not yet add your setting to one of the settings pages automatically.
   * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
   *
   * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
   */
  void addToConfig(JsonObject &root)
  {
    // Serial.println("selectedPirSetting saved");

    // JsonObject top = root.createNestedObject("UsermodPirSensor");
    // top["selectedPirSetting"] = false; //save this var persistently whenever settings are saved
  }

  /*
   * readFromConfig() can be used to read back the custom settings you added with addToConfig().
   * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
   *
   * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
   * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
   * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
   */
  void readFromConfig(JsonObject &root)
  {
    // JsonObject top = root["top"];
    // userVar0 = top["great"] | 42; //The value right of the pipe "|" is the default value in case your setting was not present in cfg.json (e.g. first boot)
  }

  /*
   * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void readFromJsonState(JsonObject &root)
  {
    // serializeJsonPretty(root, Serial);

    // make sure "motionSensingState" is actually present in state
    if (!root["motionSensingState"].isNull())
    {
      // set global var
      motionSensingGlobal = root["motionSensingState"];
    }
  }

  /*
   * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
   * Values in the state object may be modified by connected clients
   */
  void addToJsonState(JsonObject &root)
  {
    // send current variable status
    root["motionSensingState"] = motionSensingGlobal;
  }
};
