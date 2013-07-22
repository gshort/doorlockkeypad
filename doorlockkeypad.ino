#define DEBUG

#include <SPI.h>
#include <Ethernet.h>
#include <FastSPI_LED2.h>
#include <b64.h>
#include <HttpClient.h>
#include <aJSON.h>
#include <math.h>
#include <Keypad.h>

#define DOOR_PIN 9

#define NUM_LEDS 12

struct CRGB leds[NUM_LEDS];

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x39, 0x0F };
char server[] = "www.eriemakerspace.com";
char path[] = "/ems_auth.php";
IPAddress ip(192,168,0,234);
EthernetClient eClient;
const int networkTimeout = 30*1000;
const int networkDelay = 1000;

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
byte rowPins[rows] = {A3, A2, A1, A0}; //connect to the row pinouts of the keypad
byte colPins[cols] = {4, 3, 2}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

String code = "";
int codestart = 0;

void setup() {
//  LEDS.addLeds<WS2801>(leds, NUM_LEDS); //can't use the SPI pins with the ethernet shield in place
  LEDS.addLeds<WS2801, 5, 7, BGR>(leds, NUM_LEDS); //use non-spi pins in software mode...we're only driving 12 LEDs anyhow
  LEDS.showColor(CRGB(255, 100, 0)); //boot status color
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }
  delay(1000); //need a delay here for a quirky library or two
}

void loop() {
  if(code.length() <= 0) {
    status_idling();
    codestart = millis();
  } else {
    status_reading();
    if(millis() - codestart > 5000) {
      code = "";
    }
  }
  char key;
#ifdef DEBUG
  if(Serial.available()) {
    key = Serial.read();
  }
#else
  key = keypad.getKey();
#endif
  if(key) {
    if(key == '#' || key == '*') {
      if(authorize(code)) {
        digitalWrite(DOOR_PIN, HIGH); //unlock the door
        alert_allow();
        digitalWrite(DOOR_PIN, LOW); //lock the door
        code = "";
      } else {
        alert_deny();
        code = "";
      }
    } else {
      code += key;
    }
  }
}

boolean authorize(String code) { //given a PIN, check authorization against web server
  HttpClient http(eClient);
  String result;
  int err = http.get(server, path);
  if (err == 0) {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err != 200) {
      Serial.print("Bad http status: ");
      Serial.println(err);
      return(false);
    } else {
      String result = "";
      err = http.skipResponseHeaders();
      if (err >= 0) {
        int bodyLen = http.contentLength();
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ((http.connected() || http.available()) && ((millis() - timeoutStart) < networkTimeout)) {
          if (http.available()) {
            result += char(http.read());
            bodyLen--;
            timeoutStart = millis();
          } else {
            delay(networkDelay);
          }
        }
      } else {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
        return(false);
      }
    }
  } else {
    Serial.print("Connect failed: ");
    Serial.println(err);
    return(false);
  }
  http.stop();
  Serial.println(result);
  return(check_auth_result(result));
}

boolean check_auth_result(String json) {
  //parse the json and look for authorization
  return(false);
}

void status_idling() { //slowly "breathe" blue, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    LEDS.showColor(CRGB(0, 0, floor((sin(float(millis()) / 1000.0) * 0.5 + 0.5) * 255)));
  }
}

void status_reading() { //quickly "breathe" yellow, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    int v = floor((sin(float(millis()) / 100.0) * 0.5 + 0.5) * 255);
    LEDS.showColor(CRGB(v, v, 0));
  }
}

void status_thinking() { //spin a rainbow, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    fill_rainbow(leds, NUM_LEDS, float(millis()) / 4.0, 20.0);
    LEDS.show();
  }
}

void alert_deny() { //flash red three times and fade out after the third, blocking: takes roughly 1 second total
  LEDS.showColor(CRGB(255, 0, 255));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 255));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 255));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 0));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 0));
  delay(100);
  for(int ii = 255; ii >= 0; ii-=2) {
    LEDS.showColor(CRGB(ii, 0, 0));
    delay(10);
  }
}

void alert_allow() { //illuminate green and fade out after a bit, blocking: takes roughly 5 seconds total
  LEDS.showColor(CRGB(0, 255, 0));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(2500);
  for(int ii = 255; ii >= 0; --ii) {
    LEDS.showColor(CRGB(0, ii, 0));
    delay(10);
  }
}
