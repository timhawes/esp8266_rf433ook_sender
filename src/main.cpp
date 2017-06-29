/*
 * esp8266_rf433ook_sender
 * for timhawes.com/ug1 PCB
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "config.h"

const uint8_t ledPin = 12;
const uint8_t txPin = 4;

char my_id[40];
IPAddress ip;
ESP8266WebServer server(80);

void sendCustom() {
  unsigned int highOnTime = 1125;
  unsigned int highOffTime = 375;
  unsigned int lowOnTime = 375;
  unsigned int lowOffTime = 1125;
  unsigned int gapTime = 10000;
  unsigned int repeat = 10;
  unsigned int preambleHighTime = 0;
  unsigned int preambleLowTime = 0;
  char message[100] = {0};

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "highOnTime") {
      highOnTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "highOffTime") {
      highOffTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "lowOnTime") {
      lowOnTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "lowOffTime") {
      lowOffTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "gapTime") {
      gapTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "repeat") {
      repeat = server.arg(i).toInt();
    }
    if (server.argName(i) == "preambleHighTime") {
      preambleHighTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "preambleLowTime") {
      preambleLowTime = server.arg(i).toInt();
    }
    if (server.argName(i) == "message") {
      server.arg(i).toCharArray(message, sizeof(message));
    }
  }

  Serial.print("message=");
  Serial.println(message);

  digitalWrite(ledPin, HIGH);

  for (unsigned int r=0; r<repeat; r++) {
    Serial.print(".");
    if (preambleHighTime > 0 || preambleLowTime > 0) {
      digitalWrite(txPin, HIGH);
      delayMicroseconds(preambleHighTime);
      digitalWrite(txPin, LOW);
      delayMicroseconds(preambleLowTime);
    }
    for (unsigned int i=0; i<strlen(message); i++) {
      if (message[i]=='1') {
        digitalWrite(txPin, HIGH);
        delayMicroseconds(highOnTime);
        digitalWrite(txPin, LOW);
        delayMicroseconds(highOffTime);
      } else if (message[i]=='0') {
        digitalWrite(txPin, HIGH);
        delayMicroseconds(lowOnTime);
        digitalWrite(txPin, LOW);
        delayMicroseconds(lowOffTime);
      }
    }
    delayMicroseconds(gapTime);
  }
  Serial.println();

  digitalWrite(ledPin, LOW);

  server.send(200, "text/plain", "OK");

}

void sendHomeEasy2() {
  uint32_t controller = 0;
  uint8_t device = 0;
  uint8_t group = 0;
  uint8_t state = 0;
  uint8_t dim = 0;
  uint8_t repeat = 5;

  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "controller") {
      controller = server.arg(i).toInt() & 0x3ffffff; // 26 bits
    }
    if (server.argName(i) == "device") {
      device = server.arg(i).toInt() & 0xf; // 4 bits
    }
    if (server.argName(i) == "group") {
      group = server.arg(i).toInt() & 0x1; // 1 bit
    }
    if (server.argName(i) == "state") {
      state = server.arg(i).toInt() & 0x1; // 1 bit
    }
    if (server.argName(i) == "dim") {
      dim = server.arg(i).toInt() & 0xf; // 4 bits
    }
    if (server.argName(i) == "repeat") {
      repeat = server.arg(i).toInt();
    }
  }

  digitalWrite(ledPin, HIGH);

  Serial.println("sending homeeasy2");

  for (unsigned int r=0; r<repeat; r++) {

    // pre-amble
    digitalWrite(txPin, HIGH);
    delayMicroseconds(275);
    digitalWrite(txPin, LOW);
    delayMicroseconds(2675);

    // controller (26 bits)
    for (int b=25; b>=0; b--) {
      if (controller & (1 << b)) {
        // high bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
      } else {
        // low bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
      }
    }

    // group flag (1 bit)
    if (group) {
      // high bit
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(1225);
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(275);
    } else {
      // low bit
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(275);
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(1225);
    }

    if (dim > 0) {
      // special bit to enable dimming mode
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(275);
      digitalWrite(txPin, HIGH);
      delayMicroseconds(275);
      digitalWrite(txPin, LOW);
      delayMicroseconds(275);
    } else {
      // state (on-off, 1 bit)
      if (state) {
        // high bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
      } else {
        // low bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
      }
    }

    // device (4 bits)
    for (int b=3; b>=0; b--) {
      if (device & (1 << b)) {
        // high bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
      } else {
        // low bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
      }
    }

    // dim level (4 bits, hardcoded to zero)
    for (int b=3; b>=0; b--) {
      if (dim & (1 << b)) {
        // high bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
      } else {
        // low bit
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(275);
        digitalWrite(txPin, HIGH);
        delayMicroseconds(275);
        digitalWrite(txPin, LOW);
        delayMicroseconds(1225);
      }
    }

    delayMicroseconds(10000);

    if (repeat > 25) {
      yield();
    }

  }

  digitalWrite(ledPin, LOW);

  server.send(200, "text/plain", "OK");

}

void setup() {
  snprintf(my_id, sizeof(my_id), hostname_template, ESP.getChipId());

  pinMode(ledPin, OUTPUT);
  pinMode(txPin, OUTPUT);

  digitalWrite(txPin, LOW);
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print(my_id);
  Serial.print(" ");
  Serial.println(ESP.getSketchMD5());

  if (ota_enabled) {
    Serial.println("Enabling OTA updates");
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(my_id);
    if (strlen(ota_password) > 0) {
      Serial.print("OTA password: ");
      Serial.println(ota_password);
      ArduinoOTA.setPassword(ota_password);
    }
    ArduinoOTA.onStart([]() {
      Serial.println("OTA Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nOTA End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }

  wifi_station_set_hostname(my_id);
  WiFi.mode(WIFI_STA);


  server.on("/send/custom", sendCustom);
  server.on("/send/homeeasy2", sendHomeEasy2);
  server.begin();

}

void loop() {

  if (ota_enabled) {
    ArduinoOTA.handle();
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, password);

    unsigned long begin_started = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(10);
      if (millis() - begin_started > 60000) {
        ESP.restart();
      }
    }
    Serial.println("WiFi connected");
  }

  server.handleClient();

}
