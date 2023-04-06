#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "SinricPro.h"
#include "SinricProSwitch.h"

#ifndef WIFI_SSID
#define WIFI_SSID "***"
#define WIFI_PASS "***"
#endif

#define APP_KEY "***"      // O seu App Key é algo como "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "***"   // O seu App Secret é algo como "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
 
#define Quarto_ID "***"    // Algo como "5dc1564130xxxxxxxxxxxxxx"
#define Quarto_Pin 5  // O pino fisico onde está ligado
#define Sensor 15

#ifdef ENABLE_DEBUG
   #define DEBUG_ESP_PORT Serial
   #define NODEBUG_WEBSOCKETS
   #define NDEBUG
#endif 

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

bool estatusLamp = false;
unsigned long lastSensorTouch = 0;

bool QuartoState(const String &deviceId, bool &state) {
  Serial.printf("O Quarto foi %s\r\n", state?"Ligado":"Desligado");
  estatusLamp = state;
  digitalWrite(Quarto_Pin, estatusLamp);
  return true; // request handled properly
}

void handleSensorTouch() {
  unsigned long actualMillis = millis(); // get actual millis() and keep it in variable actualMillis
  
  if (digitalRead(Sensor) == HIGH  && actualMillis - lastSensorTouch > 1000)  {   
    if (estatusLamp) {     // flip estatusLamp: if it was true, set it to false, vice versa
      estatusLamp = false;
    } else {
      estatusLamp = true;
    }
    digitalWrite(Quarto_Pin, estatusLamp); // if estatusLamp indicates device turned on: turn on led (builtin led uses inverted logic: LOW = LED ON / HIGH = LED OFF)

    // get Switch device back
    SinricProSwitch& mySwitch = SinricPro[Quarto_ID];
    // send powerstate event
    mySwitch.sendPowerStateEvent(estatusLamp); // send the new powerState to SinricPro server
    Serial.printf("Device %s turned %s (manually via flashbutton)\r\n", mySwitch.getDeviceId().c_str(), estatusLamp?"Ligado":"Desligado");

    lastSensorTouch = actualMillis;  // update last button press variable
  } 
}

void setupSinricPro() {
  // add devices and callbacks to SinricPro
  SinricProSwitch& mySwitch1 = SinricPro[Quarto_ID];
  mySwitch1.onPowerState(QuartoState);
 
  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Conectado a nuvem SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Desconectado a nuvem SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(Quarto_Pin, OUTPUT);
  digitalWrite(Quarto_Pin, LOW);
  pinMode(Sensor, INPUT);

   
  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  //ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setupSinricPro(); 
}

void loop() {
  // put your main code here, to run repeatedly:

  ArduinoOTA.handle();

  handleSensorTouch();
  SinricPro.handle();
}