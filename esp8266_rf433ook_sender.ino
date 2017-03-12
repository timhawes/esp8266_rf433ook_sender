/*
 * esp8266_rf433ook_sender
 * for timhawes.com/ug1 PCB
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>.

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

const uint8_t ledPin = 12;
const uint8_t txPin = 5;

struct EepromData {
  uint8_t configured = 1;
  char ssid[128] = "";
  char passphrase[128] = "";
  char hostname[64] = "rf433ook";
} eepromData;

IPAddress ip;
ESP8266WebServer server(80);

void setup() {
  
  pinMode(ledPin, OUTPUT);
  pinMode(txPin, OUTPUT);

  digitalWrite(ledPin, HIGH);
  digitalWrite(txPin, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println("esp8266_rf433ook_sender");
  
  wifi_station_set_hostname(eepromData.hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(eepromData.ssid, eepromData.passphrase);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  
  ip = WiFi.localIP();
  Serial.print("WiFi ready: IP=");
  Serial.println(ip);

  digitalWrite(ledPin, LOW);
  
  server.on("/send/custom", sendCustom);
  server.on("/send/homeeasy2", sendHomeEasy2);
  server.begin();
  
}

void loop() {
  
  server.handleClient();
  
}

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

