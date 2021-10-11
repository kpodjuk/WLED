#include "wled.h"

/*
 * Integrated HTTP web server page declarations
 */

//Is this an IP?
bool isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

bool captivePortal(AsyncWebServerRequest *request)
{
  if (ON_STA_FILTER(request)) return false; //only serve captive in AP mode
  String hostH;
  if (!request->hasHeader("Host")) return false;
  hostH = request->getHeader("Host")->value();
  
  if (!isIp(hostH) && hostH.indexOf("wled.me") < 0 && hostH.indexOf(cmDNS) < 0) {
    DEBUG_PRINTLN("Captive portal");
    AsyncWebServerResponse *response = request->beginResponse(302);
    response->addHeader(F("Location"), F("http://4.3.2.1"));
    request->send(response);
    return true;
  }
  return false;
}

void initServer()
{
  //CORS compatiblity
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), "*");
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), "*");

 #ifdef WLED_ENABLE_WEBSOCKETS
    server.on("/liveview", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_liveviewws);
    });
 #else
    server.on("/liveview", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_liveview);
    });
  #endif
  
  //settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });
  
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!handleFileRead(request, "/favicon.ico"))
    {
      request->send_P(200, "image/x-icon", favicon, 156);
    }
  });
  
  server.on("/sliders", HTTP_GET, [](AsyncWebServerRequest *request){
    serveIndex(request);
  });
  
  server.on("/welcome", HTTP_GET, [](AsyncWebServerRequest *request){
    serveSettings(request);
  });
  
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 200,F("Rebooting now..."),F("Please wait ~10 seconds..."),129);
    doReboot = true;
  });
  
  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request){
    serveSettings(request, true);
  });

  server.on("/bulbCommand", HTTP_POST, [](AsyncWebServerRequest *request){
    serveBulbCommand(request);
  });

   server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request){
    serveSettings(request, true);
  });

  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request){
    serveJson(request);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/json", [](AsyncWebServerRequest *request) {
    bool verboseResponse = false;
    { //scope JsonDocument so it releases its buffer
      DynamicJsonDocument jsonBuffer(JSON_BUFFER_SIZE);
      DeserializationError error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
      JsonObject root = jsonBuffer.as<JsonObject>();
      if (error || root.isNull()) {
        request->send(400, "application/json", F("{\"error\":9}")); return;
      }
      fileDoc = &jsonBuffer;
      verboseResponse = deserializeState(root);
      fileDoc = nullptr;
    }
    if (verboseResponse) { //if JSON contains "v"
      serveJson(request); return; 
    } 
    request->send(200, "application/json", F("{\"success\":true}"));
  });
  server.addHandler(handler);

  server.on("/version", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)VERSION);
    });
    
  server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)millis());
    });
    
  server.on("/freeheap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", (String)ESP.getFreeHeap());
    });
  
  server.on("/irBulbControl", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_irBulbControl);
    });
    
  server.on("/url", HTTP_GET, [](AsyncWebServerRequest *request){
    URL_response(request);
    });
    
  server.on("/teapot", HTTP_GET, [](AsyncWebServerRequest *request){
    serveMessage(request, 418, F("418. I'm a teapot."), F("(Tangible Embedded Advanced Project Of Twinkling)"), 254);
    });
    
  //if OTA is allowed
  if (!otaLock){
    #ifdef WLED_ENABLE_FS_EDITOR
     #ifdef ARDUINO_ARCH_ESP32
      server.addHandler(new SPIFFSEditor(WLED_FS));//http_username,http_password));
     #else
      server.addHandler(new SPIFFSEditor("","",WLED_FS));//http_username,http_password));
     #endif
    #else
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("The FS editor is disabled in this build."), 254);
    });
    #endif
    //init ota page
    #ifndef WLED_DISABLE_OTA
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_update);
    });
    
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
      if (Update.hasError())
      {
        serveMessage(request, 500, F("Failed updating firmware!"), F("Please check your file and retry!"), 254); return;
      }
      serveMessage(request, 200, F("Successfully updated firmware!"), F("Please wait while the module reboots..."), 131); 
      doReboot = true;
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if(!index){
        DEBUG_PRINTLN(F("OTA Update Start"));
        #ifdef ESP8266
        Update.runAsync(true);
        #endif
        Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
      }
      if(!Update.hasError()) Update.write(data, len);
      if(final){
        if(Update.end(true)){
          DEBUG_PRINTLN(F("Update Success"));
        } else {
          DEBUG_PRINTLN(F("Update Failed"));
        }
      }
    });
    
    #else
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("OTA updates are disabled in this build."), 254);
    });
    #endif
  } else
  {
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 500, "Access Denied", F("Please unlock OTA in security settings!"), 254);
    });
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 500, "Access Denied", F("Please unlock OTA in security settings!"), 254);
    });
  }


    #ifdef WLED_ENABLE_DMX
    server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", PAGE_dmxmap     , dmxProcessor);
    });
    #else
    server.on("/dmxmap", HTTP_GET, [](AsyncWebServerRequest *request){
      serveMessage(request, 501, "Not implemented", F("DMX support is not enabled in this build."), 254);
    });
    #endif
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (captivePortal(request)) return;
    serveIndexOrWelcome(request);
  });

  #ifdef WLED_ENABLE_WEBSOCKETS
  server.addHandler(&ws);
  #endif
  
  //called when the url is not defined here, ajax-in; get-settings
  server.onNotFound([](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Not-Found HTTP call:");
    DEBUG_PRINTLN("URI: " + request->url());
    if (captivePortal(request)) return;

    //make API CORS compatible
    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200); return;
    }
    
    if(handleSet(request, request->url())) return;
    #ifndef WLED_DISABLE_ALEXA
    if(espalexa.handleAlexaApiCall(request)) return;
    #endif
    if(handleFileRead(request, request->url())) return;
    request->send_P(404, "text/html", PAGE_404);
  });
}


void serveIndexOrWelcome(AsyncWebServerRequest *request)
{
  if (!showWelcomePage){
    serveIndex(request);
  } else {
    serveSettings(request);
  }
}

bool handleIfNoneMatchCacheHeader(AsyncWebServerRequest* request)
{
  AsyncWebHeader* header = request->getHeader("If-None-Match");
  if (header && header->value() == String(VERSION)) {
    request->send(304);
    return true;
  }
  return false;
}

void setStaticContentCacheHeaders(AsyncWebServerResponse *response)
{
  response->addHeader(F("Cache-Control"),"no-cache");
  response->addHeader(F("ETag"), String(VERSION));
}

void serveIndex(AsyncWebServerRequest* request)
{
  if (handleFileRead(request, "/index.htm")) return;

  if (handleIfNoneMatchCacheHeader(request)) return;

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", PAGE_index, PAGE_index_L);

  response->addHeader(F("Content-Encoding"),"gzip");
  setStaticContentCacheHeaders(response);
  
  request->send(response);
}


String msgProcessor(const String& var)
{
  if (var == "MSG") {
    String messageBody = messageHead;
    messageBody += F("</h2>");
    messageBody += messageSub;
    uint32_t optt = optionType;

    if (optt < 60) //redirect to settings after optionType seconds
    {
      messageBody += F("<script>setTimeout(RS,");
      messageBody +=String(optt*1000);
      messageBody += F(")</script>");
    } else if (optt < 120) //redirect back after optionType-60 seconds, unused
    {
      //messageBody += "<script>setTimeout(B," + String((optt-60)*1000) + ")</script>";
    } else if (optt < 180) //reload parent after optionType-120 seconds
    {
      messageBody += F("<script>setTimeout(RP,");
      messageBody += String((optt-120)*1000);
      messageBody += F(")</script>");
    } else if (optt == 253)
    {
      messageBody += F("<br><br><form action=/settings><button class=\"bt\" type=submit>Back</button></form>"); //button to settings
    } else if (optt == 254)
    {
      messageBody += F("<br><br><button type=\"button\" class=\"bt\" onclick=\"B()\">Back</button>");
    }
    return messageBody;
  }
  return String();
}


void serveMessage(AsyncWebServerRequest* request, uint16_t code, const String& headl, const String& subl, byte optionT)
{
  messageHead = headl;
  messageSub = subl;
  optionType = optionT;
  
  request->send_P(code, "text/html", PAGE_msg, msgProcessor);
}


String settingsProcessor(const String& var)
{
  if (var == "CSS") {
    char buf[2048];
    getSettingsJS(optionType, buf);
    return String(buf);
  }
  
  #ifdef WLED_ENABLE_DMX

  if (var == "DMXMENU") {
    return String(F("<form action=/settings/dmx><button type=submit>DMX Output</button></form>"));
  }
  
  #endif
  if (var == "SCSS") return String(FPSTR(PAGE_settingsCss));
  return String();
}

String dmxProcessor(const String& var)
{
  String mapJS;
  #ifdef WLED_ENABLE_DMX
    if (var == "DMXVARS") {
      mapJS += "\nCN=" + String(DMXChannels) + ";\n";
      mapJS += "CS=" + String(DMXStart) + ";\n";
      mapJS += "CG=" + String(DMXGap) + ";\n";
      mapJS += "LC=" + String(ledCount) + ";\n";
      mapJS += "var CH=[";
      for (int i=0;i<15;i++) {
        mapJS += String(DMXFixtureMap[i]) + ",";
      }
      mapJS += "0];";
    }
  #endif
  
  return mapJS;
}


void serveSettings(AsyncWebServerRequest* request, bool post)
{
  byte subPage = 0;
  const String& url = request->url();
  if (url.indexOf("sett") >= 0) 
  {
    if      (url.indexOf("wifi") > 0) subPage = 1;
    else if (url.indexOf("leds") > 0) subPage = 2;
    else if (url.indexOf("ui")   > 0) subPage = 3;
    else if (url.indexOf("sync") > 0) subPage = 4;
    else if (url.indexOf("time") > 0) subPage = 5;
    else if (url.indexOf("sec")  > 0) subPage = 6;
    #ifdef WLED_ENABLE_DMX // include only if DMX is enabled
    else if (url.indexOf("dmx")  > 0) subPage = 7;
    #endif
  } else subPage = 255; //welcome page

  if (subPage == 1 && wifiLock && otaLock)
  {
    serveMessage(request, 500, "Access Denied", F("Please unlock OTA in security settings!"), 254); return;
  }

  if (post) { //settings/set POST request, saving
    if (subPage != 1 || !(wifiLock && otaLock)) handleSettingsSet(request, subPage);

    char s[32];
    char s2[45] = "";

    switch (subPage) {
      case 1: strcpy_P(s, PSTR("WiFi")); strcpy_P(s2, PSTR("Please connect to the new IP (if changed)")); forceReconnect = true; break;
      case 2: strcpy_P(s, PSTR("LED")); break;
      case 3: strcpy_P(s, PSTR("UI")); break;
      case 4: strcpy_P(s, PSTR("Sync")); break;
      case 5: strcpy_P(s, PSTR("Time")); break;
      case 6: strcpy_P(s, PSTR("Security")); strcpy_P(s2, PSTR("Rebooting, please wait ~10 seconds...")); break;
      case 7: strcpy_P(s, PSTR("DMX")); break;
    }

    strcat_P(s, PSTR(" settings saved."));
    if (!s2[0]) strcpy_P(s2, PSTR("Redirecting..."));

    if (!doReboot) serveMessage(request, 200, s, s2, (subPage == 1 || subPage == 6) ? 129 : 1);
    if (subPage == 6) doReboot = true;

    return;
  }
  
  #ifdef WLED_DISABLE_MOBILE_UI //disable welcome page if not enough storage
   if (subPage == 255) {serveIndex(request); return;}
  #endif

  optionType = subPage;
  
  switch (subPage)
  {
    case 1:   request->send_P(200, "text/html", PAGE_settings_wifi, settingsProcessor); break;
    case 2:   request->send_P(200, "text/html", PAGE_settings_leds, settingsProcessor); break;
    case 3:   request->send_P(200, "text/html", PAGE_settings_ui  , settingsProcessor); break;
    case 4:   request->send_P(200, "text/html", PAGE_settings_sync, settingsProcessor); break;
    case 5:   request->send_P(200, "text/html", PAGE_settings_time, settingsProcessor); break;
    case 6:   request->send_P(200, "text/html", PAGE_settings_sec , settingsProcessor); break;
    case 7:   request->send_P(200, "text/html", PAGE_settings_dmx , settingsProcessor); break;
    case 255: request->send_P(200, "text/html", PAGE_welcome); break;
    default:  request->send_P(200, "text/html", PAGE_settings     , settingsProcessor); 
  }
}

void serveBulbCommand(AsyncWebServerRequest *request)
{
  const String &url = request->url();

  if (url.indexOf("01") > 0)
    sendToBulbs(1);
  else if (url.indexOf("02") > 0)
    sendToBulbs(2);
  else if (url.indexOf("03") > 0)
    sendToBulbs(3);
  else if (url.indexOf("04") > 0)
    sendToBulbs(4);
  else if (url.indexOf("05") > 0)
    sendToBulbs(5);
  else if (url.indexOf("06") > 0)
    sendToBulbs(6);
  else if (url.indexOf("07") > 0)
    sendToBulbs(7);
  else if (url.indexOf("08") > 0)
    sendToBulbs(8);
  else if (url.indexOf("09") > 0)
    sendToBulbs(9);
  else if (url.indexOf("10") > 0)
    sendToBulbs(10);
  else if (url.indexOf("11") > 0)
    sendToBulbs(11);
  else if (url.indexOf("12") > 0)
    sendToBulbs(12);
  else if (url.indexOf("13") > 0)
    sendToBulbs(13);
  else if (url.indexOf("14") > 0)
    sendToBulbs(14);
  else if (url.indexOf("15") > 0)
    sendToBulbs(15);
  else if (url.indexOf("16") > 0)
    sendToBulbs(16);
  else if (url.indexOf("17") > 0)
    sendToBulbs(17);
  else if (url.indexOf("19") > 0)
    sendToBulbs(18);
  else if (url.indexOf("18") > 0)
    sendToBulbs(19);
  else if (url.indexOf("20") > 0)
    sendToBulbs(20);
  else if (url.indexOf("21") > 0)
    sendToBulbs(21);
  else if (url.indexOf("22") > 0)
    sendToBulbs(22);
  else if (url.indexOf("23") > 0)
    sendToBulbs(23);
  else if (url.indexOf("24") > 0)
    sendToBulbs(24);

  // send something so you won't get ERR_EMPTY_RESPONSE

  request->send_P(200, "text/html", "\"status\":\"ok\"");
  
}

void sendToBulbs(int commandId){
  Serial.println(commandId);

    switch (commandId)
    {
    // case 1: // brightness up
    //     irsend.sendNEC(0xFA05FF00);
    //     break;
    // case 2: // brightness down
    //     irsend.sendNEC(0xFB04FF00);
    //     break;
    // case 3: // off
    //     irsend.sendNEC(0xF906FF00);
    //     break;
    // case 4: // on
    //     irsend.sendNEC(0xF807FF00);
    //     break;
    // case 5: // red
    //     irsend.sendNEC(0xF609FF00);
    //     break;
    // case 6: // green
    //     irsend.sendNEC(0xF708FF00);
    //     break;
    // case 7: // blue
    //     irsend.sendNEC(0xF50AFF00);
    //     break;
    // case 8: // white
    //     irsend.sendNEC(0xF40BFF00);
    //     break;
    // case 9: // slightly lighter red
    //     irsend.sendNEC(0xF20DFF00);
    //     break;
    // case 10: // slightly lighter green
    //     irsend.sendNEC(0xF30CFF00);
    //     break;
    // case 11: // slightly lighter blue
    //     irsend.sendNEC(0xF10EFF00);
    //     break;
    // case 12: // flash
    //     irsend.sendNEC(0xF00FFF00);
    //     break;
    // case 13: // orange
    //     irsend.sendNEC(0xF20DFF00);
    //     break;
    // case 14: // turquoise
    //     irsend.sendNEC(0xF30CFF00);
    //     break;
    // case 15: // purple
    //     irsend.sendNEC(0xF10EFF00);
    //     break;
    // case 16: // strobe
    //     irsend.sendNEC(0xF00FFF00);
    //     break;
    // case 17: // slightly lighter orange
    //     irsend.sendNEC(0xE619FF00);
    //     break;
    // case 18: // navy
    //     irsend.sendNEC(0xE718FF00);
    //     break;
    // case 19: // pink
    //     irsend.sendNEC(0xE51AFF00);
    //     break;
    // case 20: // fade
    //     irsend.sendNEC(0xE41BFF00);
    //     break;
    // case 21: // yellow
    //     irsend.sendNEC(0xEE11FF00);
    //     break;
    // case 22: // darker navy
    //     irsend.sendNEC(0xEF10FF00);
    //     break;
    // case 23: // rose
    //     irsend.sendNEC(0xED12FF00);
    //     break;
    // case 24: // smooth
    //     irsend.sendNEC(0xEC13FF00);
    //     break;
    }
  
}