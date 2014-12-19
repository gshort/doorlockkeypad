#include <Keypad.h> // This MUST be first when building for teensy due to some header issues
#include <FastLED.h>
#include <math.h>

#define DOOR_PIN 9

#define NUM_LEDS 12

#define MAX_CODE_LEN 100

struct CRGB leds[NUM_LEDS];

const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {2, 3, 4, 5}; //brown, purple, red, white
byte colPins[cols] = {6, 7, 8}; //black, blue, orange
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

String code = String("");
unsigned long codestart = 0;
unsigned long animation_offset = 0;

void setup() {
  code.reserve(MAX_CODE_LEN);
  digitalWrite(DOOR_PIN, LOW);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.println("Ready");
  
  pinMode(DOOR_PIN, OUTPUT);
  LEDS.addLeds<WS2801, 11, 13, BGR, DATA_RATE_MHZ(1)>(leds, NUM_LEDS); //green, yellow
  LEDS.showColor(CRGB(255, 100, 0));
  
  delay(1000);
  animation_offset = 0;
}

void loop() {
  digitalWrite(DOOR_PIN, LOW); // just keep doing this forever for safety's sake
  if(animation_offset > millis()) { // need to account for millis() rolling over around 50 days. Might glitch animation for a frame
    animation_offset = 0;
  }
  if(code.length() > 0 && (millis() - codestart) > 5000) { //5-second timeout on entry
    code = "";
  }
  if(code.length() <= 0) {
    status_idling();
  } else {
    status_reading();
  }
  char key = keypad.getKey();
  if(key != NO_KEY) {
    codestart = millis(); //reset the timeout
    if(key == '#' || key == '*') { //submit for authorization on these two keys
      if(code.length() <= 3 || code.length() > MAX_CODE_LEN) { //codes have to be at least four digits
        alert_deny();
        code = "";
      } else {
        if(authorize(code)) { //authorize the code via serial
          digitalWrite(DOOR_PIN, HIGH);
          alert_allow();
          digitalWrite(DOOR_PIN, LOW);
          code = "";
        } else {
          alert_deny();
          code = "";
        }
      }
    } else { //for all non # or * keys, just append it to the currently-entered code
      code += key;
      Serial.println(code);
      LEDS.showColor(CRGB(255, 255, 255));
      delay(200);
    }
  }
}

boolean authorize(String code) { //given a PIN, check authorization via serial
  //Serial.println(code);
  // temporary hard-coded code
  if(code.equals("1593570")) {
    return(true);
  } else {
    return(false);
  }
  /*
  status_thinking();
  Serial.println("Code: " + code);
  while(!Serial.available()) {
    status_thinking();
  }
  char r = Serial.read();
  if(r == 'y') {
    return(true);
  }
  return(false);
  */
}

void status_idling() { //slowly "breathe" blue, single frame per call
  if((millis() - animation_offset) % 10 == 0) {
    LEDS.showColor(CRGB(0, 0, floor((sin(float(millis() - animation_offset) / 1000.0) * 0.5 + 0.5) * 255)));
  }
}

void status_reading() { //spin a rainbow, single frame per call
  if((millis() - animation_offset) % 10 == 0) {
    fill_rainbow(leds, NUM_LEDS, float(millis() - animation_offset) / 4.0, 20.0);
    LEDS.show();
  }
}

void status_thinking() { //quickly "breathe" yellow, single frame per call
  if((millis() - animation_offset) % 10 == 0) {
    int v = floor((sin(float(millis() - animation_offset) / 100.0) * 0.5 + 0.5) * 255);
    LEDS.showColor(CRGB(v, v, 0));
  }
}

void alert_deny() { //flash red three times and fade out after the third, blocking during animation, takes roughly 1
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
  for(int ii = 255; ii >= 0; ii-=4) {
    LEDS.showColor(CRGB(ii, 0, 0));
    delay(10);
  }
  animation_offset = millis();
}

void alert_allow() { //illuminate green and fade out after a bit, blocking during animation, takes roughly 5
  LEDS.showColor(CRGB(0, 255, 0));
  delay(2500);
  for(int ii = 255; ii >= 0; --ii) {
    LEDS.showColor(CRGB(0, ii, 0));
    delay(10);
  }
  animation_offset = millis();
}
