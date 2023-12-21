#pragma once

#include "wled.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include "sendButtonPressToLightbulb.h"


// ################################## USEFUL DEFINES ##################################
#define ENABLE_WEBSERIAL 0 // enable optional webserial support
#define SHINE_INTERNAL_LED 0 // always shine internal LED when turned on (usefull to know if board gets power)
#define DEBUG_PRINT_SENT_COMMANDS 0 // print all commands sent to lightbulbs


#if ENABLE_WEBSERIAL == 1// WIFI DEBUG
#include <WebSerial.h>
void recvMsg(uint8_t *data, size_t len)
{
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  WebSerial.println(d);
}
#endif

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

// class name. Use something descriptive and leave the ": public Usermod" part :)
class UserModIrBulbControl : public Usermod
{
private:
  // Private class members. You can declare variables and functions only accessible to your usermod here

  bool globalAutoColor = true; // should auto color approximation be turned on by default?
  int32_t previousCommandRepeat;
  int32_t currentCommandRepeat;
  uint8_t lastCommand;

  // *********** CONSTANTS ***********
  const uint8_t repeats = 2; // how many times to repeat communication of each command, for better reliabilty

  // WTF???
  const bool makeSureCommandsAreReceived = false; // should last command be repeated once every X ms?
  // not sure what's wrong☝️👆, board crashes when it's == true
  // don't use this!
  const int16_t commandRepeatInterval_ms = 1500; // how many ms to wait before repeating command

  // For color approximation: Which colors you have on buttons
  const int distinctRGB[14][3] = {
      {0, 0, 0},       // black
      {255, 255, 255}, // white
      {186, 3, 252},   // light_purple
      {133, 0, 181},   // purple
      {89, 0, 140},    // dark_purple
      {30, 255, 0},    // light green
      {0, 156, 13},    // green
      {0, 106, 227},   // light_blue
      {25, 25, 196},   // blue
      {0, 0, 255},     // dark_blue
      {255, 221, 0},   // light_yellow
      {255, 208, 0},   // yellow
      {255, 162, 0},   // dark yellow
      {255, 0, 0}      // red
  };

  // Which strings will be the output
  const String distinctColors[14] =
      {
          "black",
          "white",
          "light_purple",
          "purple",
          "dark_purple",
          "light_green",
          "green",
          "light_blue",
          "blue",
          "dark_blue",
          "light_yellow",
          "yellow",
          "dark_yellow",
          "red"};

public:
  // Functions called by WLED

  /*
   * setup() is called once at boot. WiFi is not yet connected at this point.
   * You can use it to initialize variables, sensors or similar.
   */
  void setup()
  {
    // Serial.println("Hello from UserModIrBulbControl!");
    irsend.begin();

#if SHINE_INTERNAL_LED == 1
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
#endif
    
    // sendButtonPressToLightbulb(4); // Turn on at the very beggining
    // sendButtonPressToLightbulb(2); // bright up
    // sendButtonPressToLightbulb(2); // bright up
    // sendButtonPressToLightbulb(2); // bright up
    // sendButtonPressToLightbulb(2); // bright up
    // sendButtonPressToLightbulb(4); // Turn off

#if ENABLE_WEBSERIAL == 1
    WebSerial.begin(&server);
    WebSerial.msgCallback(recvMsg);
#endif
  }

  /*
   * connected() is called every time the WiFi is (re)connected
   * Use it to initialize network interfaces
   */
  void connected()
  {
    // Serial.println("Connected to WiFi!");
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
    // repeat last command from time to time
    if (makeSureCommandsAreReceived)
    {
      currentCommandRepeat = millis();
      if (currentCommandRepeat - previousCommandRepeat > commandRepeatInterval_ms)
      {
        previousCommandRepeat = currentCommandRepeat;
        Serial.printf("Repeating cmd ID=%i\n", lastCommand);
        sendButtonPressToLightbulb(lastCommand);
      }
    }
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
    // send current variable status
    root["autoIRcolorState"] = globalAutoColor;
  }

  String closestColor(int r, int g, int b)
  {
    String colorReturn = "NA";
    int biggestDifference = 1000;
    for (int i = 0; i < 14; i++)
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

    // Serial.printf("R:%i", red);
    // Serial.printf(" G:%i", green);
    // Serial.printf(" B:%i\n", blue);

    // make sure "bulbCommand" is actually present in state
    if (!root["bulbCommand"].isNull())
    {
      uint8_t bulbCommand = root["bulbCommand"]; // Serve bulbCommand
      sendButtonPressToLightbulb(bulbCommand);
    }

    // make sure "autoIRcolorState" is actually present in state
    if (!root["autoIRcolorState"].isNull())
    {
      // set global var
      globalAutoColor = root["autoIRcolorState"];
    }

    if (globalAutoColor)
    {
      // find closest color when state is changed
      serveClosestColor(root);
    }
  }

  void serveClosestColor(JsonObject &root)
  {

    uint8_t red = root["seg"]["col"][0][0]; // segment 0 id 0 color 0
    uint8_t green = root["seg"]["col"][0][1];
    uint8_t blue = root["seg"]["col"][0][2];

    // serve finding closest color
    // Serial.println("Found closest color:");
    String distinctColor = closestColor(red, green, blue);
    // Serial.println(distinctColor);

    // send buttonpresses according to found color
    if (distinctColor == "black")
    {
      // sendButtonPressToLightbulb(3);
    }
    else if (distinctColor == "white")
    {
      sendButtonPressToLightbulb(8);
    }
    else if (distinctColor == "light_purple")
    {
      sendButtonPressToLightbulb(23);
    }
    else if (distinctColor == "purple")
    {
      sendButtonPressToLightbulb(15);
    }
    else if (distinctColor == "dark_purple")
    {
      sendButtonPressToLightbulb(11);
    }
    else if (distinctColor == "light_green")
    {
      sendButtonPressToLightbulb(10);
    }
    else if (distinctColor == "green")
    {
      sendButtonPressToLightbulb(6);
    }
    else if (distinctColor == "light_blue")
    {
      sendButtonPressToLightbulb(19);
    }
    else if (distinctColor == "blue")
    {
      sendButtonPressToLightbulb(22);
    }
    else if (distinctColor == "dark_blue")
    {
      sendButtonPressToLightbulb(7);
    }
    else if (distinctColor == "light_yellow")
    {
      sendButtonPressToLightbulb(21);
    }
    else if (distinctColor == "yellow")
    {
      sendButtonPressToLightbulb(17);
    }
    else if (distinctColor == "dark_yellow")
    {
      sendButtonPressToLightbulb(9);
    }
    else if (distinctColor == "red")
    {
      sendButtonPressToLightbulb(5);
    }
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

  void sendButtonPressToLightbulb(uint8_t button)
  {

#if DEBUG_PRINT_SENT_COMMANDS == 1
    Serial.printf("Sending ID=%i to bulb\n", button);
#endif

    switch (button)
    {
    case 1: // brightness up (there are 4 brightness levels)
      irsend.sendNEC(reverseBits(0xFA05FF00, 32), 32, repeats);
      break;
    case 2: // brightness down
      irsend.sendNEC(reverseBits(0xFB04FF00, 32), 32, repeats);
      break;
    case 3: // off
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
    // save last command
    if (makeSureCommandsAreReceived)
      lastCommand = button;
  }
};
