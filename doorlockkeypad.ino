#include <FastSPI_LED2.h>
#include <math.h>
#include <Keypad.h>

#define DOOR_PIN 9

#define NUM_LEDS 12

struct CRGB leds[NUM_LEDS];

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
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  LEDS.addLeds<WS2801, 5, 7, BGR>(leds, NUM_LEDS); //use non-spi pins in software mode...we're only driving 12 LEDs anyhow
  LEDS.showColor(CRGB(255, 100, 0));
  
  delay(1000);
}

void loop() {
  char key = keypad.getKey();
  if(key) {
    codestart = millis(); //reset the timeout
    if((key == '#' || key == '*') && code.length() > 0) {
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
  if(code.length() <= 0) {
    status_idling();
  } else {
    if(millis() - codestart > 5000) {
      status_idling();
      code = "";
      codestart = millis();
    } else {
      status_reading();
    }
  }
}

boolean authorize(String code) { //given a PIN, check authorization against web server
  status_thinking();
  Serial.println("Code: " + code);
  while(!Serial.available()) {
    status_thinking();
  }
  return(Serial.read());
}

void status_idling() { //slowly "breathe" blue, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    LEDS.showColor(CRGB(0, 0, floor((sin(float(millis()) / 1000.0) * 0.5 + 0.5) * 255)));
  }
}

void status_reading() { //spin a rainbow, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    fill_rainbow(leds, NUM_LEDS, float(millis()) / 4.0, 20.0);
    LEDS.show();
  }
}

void status_thinking() { //quickly "breathe" yellow, deterministic: call as often as convenient (or possible)
  if(millis() % 10 == 0) {
    int v = floor((sin(float(millis()) / 100.0) * 0.5 + 0.5) * 255);
    LEDS.showColor(CRGB(v, v, 0));
  }
}

void alert_deny() { //flash red three times and fade out after the third, blocking: takes roughly 1 second total
  LEDS.showColor(CRGB(255, 0, 0));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 0));
  delay(100);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(50);
  LEDS.showColor(CRGB(255, 0, 0));
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
