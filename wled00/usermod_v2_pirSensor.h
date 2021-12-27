#pragma once

#include "wled.h"

class UsermodPirSensor : public Usermod
{
private:
public:
  void setup()
  {
    // Serial.println("Hello from usermod pir sensor!");
  }

  void connected()
  {
    // Serial.println("WIFI CONNECTED!");
  }

  void loop()
  {
  }
};