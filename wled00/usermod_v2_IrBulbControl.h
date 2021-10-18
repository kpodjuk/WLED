#pragma once

#include "wled.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include "sendButtonPressToLightbulb.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * This is an example for a v2 usermod.
 * v2 usermods are class inheritance based and can (but don't have to) implement more functions, each of them is shown in this example.
 * Multiple v2 usermods can be added to one compilation easily.
 * 
 * Creating a usermod:
 * This file serves as an example. If you want to create a usermod, it is recommended to use usermod_v2_empty.h from the usermods folder as a template.
 * Please remember to rename the class and file to a descriptive name.
 * You may also use multiple .h and .cpp files.
 * 
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */
const uint16_t kIrLed = 4; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);     // Set the GPIO to be used to sending the message.

//class name. Use something descriptive and leave the ": public Usermod" part :)
class UserModIrBulbControl : public Usermod
{
private:
  //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;

public:
  //Functions called by WLED

  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    Serial.println("Hello from UserModIrBulbControl!");
    irsend.begin();
    sendButtonPressToLightbulb(3); // Turn off at the very beggining
  }

  /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
  void connected()
  {
    Serial.println("Connected to WiFi!");
  }

  /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
  void loop()
  {
    // horrible solution, instead of calling "sendButtonPressToLightbulb" from wled_server.cpp
    // I'm passing the data through bulbCommand global variable
    // if (millis() - lastTime > 500)
    // {
    //   if (bulbCommand > -1)
    //   {
    //     sendButtonPressToLightbulb(bulbCommand);
    //     Serial.println(bulbCommand);
    //     bulbCommand = -1; // -1 means there's no command to fullfil
    //   }

    //   lastTime = millis();
    // }
  }

  /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
  /*
    void addToJsonInfo(JsonObject& root)
    {
      int reading = 20;
      //this code adds "u":{"Light":[20," lux"]} to the info object
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(reading); //value
      lightArr.add(" lux"); //unit
    }
    */

  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void addToJsonState(JsonObject &root)
  {
    // root["IrBulbControlCommand"] = 0xFF;

  }


  String closestColor(int r, int g, int b)
  {
  const int distinctRGB[22][3] = {
      {255, 255, 255},
      {0, 0, 0},
      {128, 0, 0},
      {255, 0, 0},
      {255, 200, 220},
      {170, 110, 40},
      {255, 150, 0},
      {255, 215, 180},
      {128, 128, 0},
      {255, 235, 0},
      {255, 250, 200},
      {190, 255, 0},
      {0, 190, 0},
      {170, 255, 195},
      {0, 0, 128},
      {100, 255, 255},
      {0, 0, 128},
      {67, 133, 255},
      {130, 0, 150},
      {230, 190, 255},
      {255, 0, 255},
      {128, 128, 128}};
  const String distinctColors[22] =
      {"white",
       "black",
       "maroon",
       "red",
       "pink",
       "brown",
       "orange",
       "coral",
       "olive",
       "yellow",
       "beige",
       "lime",
       "green",
       "mint", "teal",
       "cyan",
       "navy",
       "blue",
       "purple",
       "lavender",
       "magenta",
       "grey"};



    String colorReturn = "NA";
    int biggestDifference = 1000;
    for (int i = 0; i < 22; i++)
    {
      if (sqrt(pow(r - distinctRGB[i][0], 2) + pow(g - distinctRGB[i][1], 2) + pow(b - distinctRGB[i][2], 2)) < biggestDifference)
      {
        colorReturn = distinctColors[i];
        biggestDifference = sqrt(pow(r - distinctRGB[i][0], 2) + pow(g - distinctRGB[i][1], 2) + pow(b - distinctRGB[i][2], 2));
      }
    }
    return colorReturn;
}


  /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void readFromJsonState(JsonObject &root)
  {
    // userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
    // if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));

    uint8_t red = root["seg"]["col"][0][0]; // segment 0 id 0 color 0
    uint8_t green = root["seg"]["col"][0][1];
    uint8_t blue = root["seg"]["col"][0][2];
// StaticJsonBuffer<200> jsonBuffer;
// root = jsonBuffer.createObject();
    Serial.println(closestColor(red, green, blue));

  // StaticJsonDocument<100> testDocument;


    // root.printTo(Serial);
  // char buffer[500];

  // serializeJsonPretty(red, buffer);

// Serial.println(buffer);
    // Serial.print(serializeJsonPretty(root.));
    // Serial.println("#################");
    // serializeJsonPretty(root, Serial);
    // Serial.println("#################");

    // serializeJsonPretty(root["seg"]["col"][0][0], Serial);
    // Serial.printf("R:%i", red);
    // Serial.printf(" G:%i", green);
    // Serial.printf(" B:%i", blue);
    // Serial.printf("\n", blue);

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
    // JsonObject top = root.createNestedObject("exampleUsermod");
    // top["great"] = userVar0; //save this var persistently whenever settings are saved
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
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return USERMOD_ID_EXAMPLE;
  }

  //More methods can be added in the future, this example will then be extended.
  //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};

void sendButtonPressToLightbulb(unsigned int button)
{
  const unsigned char repeats = 2; // how many times to repeat communication of each command, for better reliabilty
  if (button != 3 && button != 1 && button != 2)
  {
    // try to turn ON as a first thing in almost every case, unless command is to go black
    // or increase/decrease brightness (we don't want to turn on in that case, we don't know the choosen color so the UI would be inconsistent)
    irsend.sendNEC(reverseBits(0xF807FF00, 32), 32, repeats);
  }
  switch (button)
  {
  case 1: // brightness up (there are 4 brightness levels)
    irsend.sendNEC(reverseBits(0xFA05FF00, 32), 32, repeats);
    break;
  case 2: // brightness down
    irsend.sendNEC(reverseBits(0xFB04FF00, 32), 32, repeats);
    break;
  case 3: // black
    irsend.sendNEC(reverseBits(0xF906FF00, 32), 32, repeats);
    break;
  case 4: // on
    irsend.sendNEC(reverseBits(0xF807FF00, 32), 32, repeats);
    break;
  case 5: // red
    irsend.sendNEC(reverseBits(0xF609FF00, 32), 32, repeats);
    break;
  case 6: // green
    irsend.sendNEC(reverseBits(0xF708FF00, 32), 32, repeats);
    break;
  case 7: // blue
    irsend.sendNEC(reverseBits(0xF50AFF00, 32), 32, repeats);
    break;
  case 8: // white
    irsend.sendNEC(reverseBits(0xF40BFF00, 32), 32, repeats);
    break;
  case 9:                                                     // slightly lighter red
    irsend.sendNEC(reverseBits(0xF20DFF00, 32), 32, repeats); // duplicate with 13 - same hex
    break;
  case 10: // slightly lighter green
    irsend.sendNEC(reverseBits(0xF30CFF00, 32), 32, repeats);
    break;
  case 11: // slightly lighter blue
    irsend.sendNEC(reverseBits(0xF10EFF00, 32), 32, repeats);
    break;
  case 12:                                                    // flash
    irsend.sendNEC(reverseBits(0xF00FFF00, 32), 32, repeats); // duplicate with 16 - same hex
    break;
  case 13: // orange
    irsend.sendNEC(reverseBits(0xF20DFF00, 32), 32, repeats);
    break;
  case 14: // turquoise
    irsend.sendNEC(reverseBits(0xF30CFF00, 32), 32, repeats);
    break;
  case 15: // purple
    irsend.sendNEC(reverseBits(0xF10EFF00, 32), 32, repeats);
    break;
  case 16: // strobe
    irsend.sendNEC(reverseBits(0xF00FFF00, 32), 32, repeats);
    break;
  case 17: // slightly lighter orange
    irsend.sendNEC(reverseBits(0xE619FF00, 32), 32, repeats);
    break;
  case 18: // navy
    irsend.sendNEC(reverseBits(0xE718FF00, 32), 32, repeats);
    break;
  case 19: // pink
    irsend.sendNEC(reverseBits(0xE51AFF00, 32), 32, repeats);
    break;
  case 20: // fade
    irsend.sendNEC(reverseBits(0xE41BFF00, 32), 32, repeats);
    break;
  case 21: // yellow
    irsend.sendNEC(reverseBits(0xEE11FF00, 32), 32, repeats);
    break;
  case 22: // darker navy
    irsend.sendNEC(reverseBits(0xEF10FF00, 32), 32, repeats);
    break;
  case 23: // rose
    irsend.sendNEC(reverseBits(0xED12FF00, 32), 32, repeats);
    break;
  case 24: // smooth
    irsend.sendNEC(reverseBits(0xEC13FF00, 32), 32, repeats);
    break;
  }
}