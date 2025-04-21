/*
 * Firmware for FancyDimmer
 */

#include <EEPROM.h>

const uint8_t EEPROM_MARKER_B2 = 0xA5;
const uint8_t EEPROM_MARKER_B3 = 0x5A;
const uint8_t EEPROM_ADDR_B2   = 0;
const uint8_t EEPROM_ADDR_B3   = 4;



uint32_t lastButton1Time = 0;
uint8_t  button1ClickCount = 0;
bool lastB1State = false;
bool ispressed = false;
uint16_t value = 0;
uint16_t pwmWert = 0;
int pwmold = 0;
int poti = 0;
int newpwm = 0;
int oldpwm = 0;
uint16_t smoothed = 0;



const uint16_t PWM_TOP = 668;//30khz
const uint8_t button1 = PIN_PA1;
const uint8_t button2 = PIN_PA2;
const uint8_t button3 = PIN_PA3;

void blinkPWM(uint8_t times, uint16_t del) {
  for (uint8_t i = 0; i < times; i++) {
    TCA0.SINGLE.CMP0 = 0;         // LED aus
    delay(del);
    TCA0.SINGLE.CMP0 = PWM_TOP;  // LED an
    delay(del);
  }
}

void writePWMtoEEPROM(uint8_t slotAddr, uint8_t marker, uint16_t value) {
  EEPROM.update(slotAddr, marker);
  EEPROM.update(slotAddr + 1, value >> 8);
  EEPROM.update(slotAddr + 2, value & 0xFF);
}

bool readPWMfromEEPROM(uint8_t slotAddr, uint8_t marker, uint16_t &valueOut) {

    uint8_t high = EEPROM.read(slotAddr + 1);
    uint8_t low  = EEPROM.read(slotAddr + 2);
    valueOut = ((uint16_t)high << 8) | low;

}

void clearEEPROM() {
  for (uint8_t i = 0; i < 6; i++) EEPROM.update(i, 0xFF);
}



void setup() {
TCA0.SINGLE.CMP0 = 0;

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);

  TCA0.SPLIT.CTRLA = 0;

  PORTA.DIRSET = PIN7_bm;

  TCA0.SINGLE.CTRLA = 0;
  TCA0.SPLIT.CTRLA  = 0;
  TCA0.SPLIT.CTRLB  = 0;
  TCA0.SINGLE.CNT   = 0;
  TCA0.SPLIT.CTRLD  = 0;
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;
  TCA0.SINGLE.PER = PWM_TOP;
  TCA0.SINGLE.CMP0 = 0;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}

void loop() {


int raw = ((uint32_t)analogRead(PIN_PA6) * PWM_TOP) / 1023;
bool b1 = digitalRead(button1);
bool b2 = digitalRead(button2);
bool b3 = digitalRead(button3);

if((abs(oldpwm - raw)) > 2){
  pwmWert = PWM_TOP - raw;
  oldpwm = raw; 
}
else {
  pwmWert = PWM_TOP - oldpwm ;
}





if (b1 && !lastB1State) {
  uint32_t now = millis();
  if (now - lastButton1Time < 2000) {
    button1ClickCount++;
  } else {
    button1ClickCount = 1;
  }
  lastButton1Time = now;

  if (button1ClickCount >= 5) {
    clearEEPROM(); 
    delay(500);
    blinkPWM(5, 500); 
    button1ClickCount = 0;
  }
}
lastB1State = b1;

if(b2 && b1) {
  writePWMtoEEPROM(EEPROM_ADDR_B2, EEPROM_MARKER_B2, pwmWert);
  blinkPWM(2, 1000);
}
else if(b3 && b1) {
  writePWMtoEEPROM(EEPROM_ADDR_B3, EEPROM_MARKER_B3, pwmWert);
  blinkPWM(2, 1000);
  
}
else if(b2) {
  readPWMfromEEPROM(EEPROM_ADDR_B2, EEPROM_MARKER_B2, value);
  pwmold = pwmWert;
  ispressed = true;
  blinkPWM(3, 100);
  
}
else if(b3) {
  readPWMfromEEPROM(EEPROM_ADDR_B3, EEPROM_MARKER_B3, value);
  pwmold = pwmWert;
  ispressed = true;
  blinkPWM(3, 100);
  
}


if(ispressed) {
  TCA0.SINGLE.CMP0 = value;
  if(abs(pwmold - pwmWert) >= 3){
    ispressed = false;
    pwmold = pwmWert;
  }
  return;
}

TCA0.SINGLE.CMP0 = pwmWert;

}
