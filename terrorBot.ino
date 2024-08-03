/*
---------------------------------------
|       TERRORBOT FIRMWARE v2.0       |                                                               05/2016
---------------------------------------

    TERRORBOT FOR ORANGE DARK TERROR HEAD

    copyright Michael Karsay 2013 - 2016
    http://beam.to/terrorBot
    
    You may use and/or modify this code for personal use.
    If you want to publish modified versions of this code,
    keep the header with my name and homepage intact and give me proper credits.   
    Feel free to contact me if you need other agreements.

    FEATURING:
    ----------
    
    - INDIVIDUAL SERVO SPEEDs

    - 12 PRESET SLOTS IN 4 BANKS
      --> DOUBLE LONGPRESS BUTTON 2 & 3 WILL CHANGE BANK
      
    - NO LAG EVENTS (BYPASSED CLICKBUTTON LIBRARY)
      --> ALLOW IMEDIATE BUTTON RESPONSE WHILE SWITCHING PRESETS FROM BTN3 TO BTN2 AND BACK IN BANK 3 + 4 

    - VOLUME AND GAIN BOOST
      --> LONGPRESS BUTTON 2 WILL BOOST VOLUME
      --> LONGPRESS BUTTON 3 WILL BOOST GAIN
        --> DOUBLE CLICK BUTTON 2 WHILE IN EDITMODE TO SET VOLUME VALUES (BOOST-AMMOUNT + SPEED)
        --> DOUBLE CLICK BUTTON 3 WHILE IN EDITMODE TO SET GAIN VALUES (BOOST-AMMOUNT + SPEED)
 
     - EDIT MODE
      --> LONGPRESS BUTTON 1 WILL GO TO EDIT MODE FOR ACTIVE PRESET
          -- BUTTON 2 AND 3 WILL CHANGE THE VALUE UP AND DOWN
             - BUTTON LEDs WILL INDICATE THE HEIGH OF THE VALUE
          -- SINGLE CLICK ON BUTTON 1 WILL JUMP TO THE NEXT STATE
          -- LONG CLICK BUTTON 1 WILL JUMP TO PREVIOUS STATE
          -- TRIPPLE CLICK BUTTON 1 WILL EXIT WITHOUT SAVING
          -- STATES:
             -STATE-1: POSITION VOLUME
             -STATE-2: POSITION SHAPE
             -STATE-3: POSITION GAIN
             -STATE-4: SPEED FOR VOLUME SERVO
             -STATE-5: SPEED FOR SHAPE SERVO
              --> THERE IS A TIMER HERE. IF NO BUTTON IS PRESSED FOR 5 SECONDS
                  ALL SERVOS WILL GET THE SAME SPEED AS SELECTED FOR VOLUME IN STATE-4.                  
             -STATE-6: SPEED FOR GAIN SERVO
             -STATE-7: SAVE AND EXIT

    - COPY PRESET TO ANOTHER SLOT
      --> LOAD PRESET YOU WANT TO COPY AND ENTER EDITMODE
          -- TRIPPLE CLICK ON BUTTON 3 WHILE IN EDITSTATE 1 WILL LAUNCH NEW SLOT SELECTOR
          -- TRIIPLE CLICK ON BUTTON 1 TO CANCEL
          -- IF NO BUTTON IS PRESSED FOR 5 SECONDS SELECTOR WILL BE CANCELED

    - BACKUP AND RESTORE PRESETS
      --> HOLD BUTTON 1 DURING STARTUP FOR BACKUP
          -- THIS WILL SERIAL PRINT ALL VALUES IN AN ARRAY AND
             BACKUP ALL VALUES TO HIGHER EEPROM SLOTS
      --> HOLD BUTTON 2 AND 3 DURING STARTUP
             TO RESTORE VALUES FROM HIGHER EEPROM SLOTS

    - GLOBAL OFFSET
      --> DOUBLE CLICK BUTTON 1 WHILE IN EDITMODE 1 TO SET MASTER VOLUME
          -- HOLD BUTTON 2 AND 3 TO CHANGE
          -- PRESS BUTTON 1 TO SET SHAPE OFFSET
          -- PRESS BUTTON 1 AGAIN TO SET GAIN OFFSET
          -- PRESS BUTTON 1 AGAIN TO SAVE AND EXIT
      --> HOLD BUTTON2 DURING STARTUP TO RESET VALUES
          
    - SYSTEM SETTINGS
      --> TRIPPLE CLICK BUTTON 2 WHILE IN EDITMODE 1 TO SET SYSTEM SETTINGS
          -- SET longClickTime_default (TIME UNTIL CLICK AND HOLD BUTTON IS A LONGPRESS)
          -- HOLD BUTTON 2 AND 3 TO CHANGE VALUE 
          --> PRESS BUTTON 1 TO SET easeFreemode (SPEED OF THE SERVOS IN FREEMODE)
            -- TRIPPLE PRESS BUTTON 2 HERE TO RESTORE DEFAULT VALUES AND EXIT
            -- PRESS BUTTON 1 AGAIN TO SAVE CHANGES AND EXIT
          --> TRIPPLE LONG PRESS BUTTON 3 INSTEAD TO ENTER EEPROM ADRESS SHIFT MENU
            -- PRESS BUTTON 2 TO SHIFT EEPROM ADDRESSES ONE BLOCK
            -- PRESS BUTTON 1 OR 3 TO CANCEL
            
    - FREEMODE
      --> TRIPPLE CLICK ON DESIRED PRESET TO OPEN IT IN FREEMODE
          -- CONTROLL EACH SERVO WITH A SINGLE BUTTON
           - PRESS AND HOLD BUTTON 1 WILL CHANGE VOLUME
           - PRESS AND HOLD BUTTON 2 WILL CHANGE SHAPE
           - PRESS AND HOLD BUTTON 3 WILL CHANGE GAIN
           - SINGLE CLICK CHANGES EACH DIRECTION (LED ON = INCREASE VALUE)
           - TRIPPLE CLICK BUTTON 2 TO EXIT AND SAVE TO ACTUAL PRESET
           - TRIPPLE CLICK BUTTON 1 TO EXIT WITHOUT SAVING AND RETURN TO ACTUAL PRESET 
           - TRIPPLE CLICK BUTTON 3 TO EXIT WITHOUT SAVING AND RETURN TO PRESET WITH BOOST SETTINGS

    - METRONOME
      --> TRIPPLE LONG CLICK TO ACTIVATE (CLICK - CLICK - CLICK AND HOLD)
          -- HOLD BUTTON 2 AND 3 TO CHANGE SPEED
          -- PRESS BUTTON 1 TO EXIT
          
-------------------------------------------------------------------------------------------------------------
*/
#include <VarSpeedServo.h>
#include <EEPROM.h>
#include <ClickButton.h>
int adr = 88;
int eepromLevelling = 0;
int ee;
// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------- SERVOs
VarSpeedServo VolumeServo;
VarSpeedServo ShapeServo;
VarSpeedServo GainServo;
const int VolumePin =                 9;
const int ShapePin =                 10;
const int GainPin =                  11;
int volume;
int shape;
int gain;
int masterOffset =                    0;
int shapeOffset =                     0;
int gainOffset =                      0;
int gainBoost =                      27;
int gainBoostCount =                  0;
int volumeBoost =                    27;
int volumeBoostCount =                0;
int speedVolumeboost =                7;
int speedGainboost =                  7;
int lastVolume;
int lastShape;
int lastGain;
int volumeSpeed =                    27; // sweep speed, 1 is slowest, 255 fastest)
int shapeSpeed =                     27;
int gainSpeed =                      27;
int editSpeed =                     255;
int volumeMin =                       7; // range of the servos
int volumeMax =                     174;
int shapeMin =                        4;
int shapeMax =                      165;
int gainMin =                         7;
int gainMax =                       158;
// -------------------------------------------------------------------------------------------------- BUTTONs
int function =                        0;
const int buttons =                   3;
const int buttonPin1 =                0;
const int buttonPin2 =                8;
const int buttonPin3 =               12;
ClickButton button[3] = {
  ClickButton (buttonPin1, LOW, CLICKBTN_PULLUP),
  ClickButton (buttonPin2, LOW, CLICKBTN_PULLUP),
  ClickButton (buttonPin3, LOW, CLICKBTN_PULLUP),
};
int pressed[buttons]  =     { 0, 0, 0 };
long longClickTime_default = 1720;
long longClickTimeZ;
// ----------------------------------------------------------------------------------------------------- LEDs
const int ledPin[buttons] = { 2, 4, 7 };    // Arduino pins to the LEDs
const int R_PIN =                     3;
const int G_PIN =                     5;
const int B_PIN =                     6;
int r;
int g;
int b;
// -------------------------------------------------------------- INDICATOR LEVEL TO TRIGGER C1, C2 & C3 LEDs
int volumeIndicator1 =               66;
int volumeIndicator2 =              127;
int volumeIndicator3 =              158;
int shapeIndicator1 =                99;
int shapeIndicator2 =               150;
int shapeIndicator3 =               161;
int gainIndicator1 =                 27;
int gainIndicator2 =                 88;
int gainIndicator3 =                127;
int servoSpeedsIndicator1 =          27;
int servoSpeedsIndicator2 =         127;
int servoSpeedsIndicator3 =         172;
// ----------------------------------------------------------------------------------------------------- VARs
int DELAY =                          10;  // Delay per loop in ms
float easeFreemode =                 35;  // Free Mode Servospeed factor
int editState =                       0;
int oldEditState;
int bank =                            1;
int oldBank;
int state =                           1;
int oldState =                        0;
int lastBtn =                         0;
int timer =                           0;
int blinker =                         0;
int tempo =                         400;  // tempo 400 will be arround 120 bpm
int canelDelay =                    400; // time until all servos will 
float y;                                   // get the value of the first in edit when switching to shapeSpeed
float x =                             1;
boolean trigger =                 false;
boolean DirectionVolume =          true;
boolean DirectionShape =           true;
boolean DirectionGain =            true;
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------- VOID SETUP
// ----------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
// --------------------------------------------------------------------------------------- READ SYSTEM VALUES
  eepromLevelling = EEPROM.read(1020);
  delay(100);
  ee = eepromLevelling * adr;
  delay(10);
  long four = EEPROM.read(ee + 82);
  long three = EEPROM.read(ee + 83);
  long two = EEPROM.read(ee + 84);
  long one = EEPROM.read(ee + 85);    
  longClickTime_default = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
  easeFreemode = EEPROM.read(ee + 86);
// ---------------------------------------------------------------------------------------- READ BOOST VALUES
  volumeBoost = EEPROM.read(ee + 72);
  speedVolumeboost = EEPROM.read(ee + 73);
  gainBoost = EEPROM.read(ee + 74);
  speedGainboost = EEPROM.read(ee + 75);
  int m = EEPROM.read(ee + 76);
  int mO = EEPROM.read(ee + 77);
  if (m == 1)
    masterOffset = mO;
  else
    masterOffset = -mO;    
  int s = EEPROM.read(ee + 78);
  int sO = EEPROM.read(ee + 79);
  if (s == 1)
    shapeOffset = sO;
  else
    shapeOffset = -sO;    
  int g = EEPROM.read(ee + 80);
  int gO = EEPROM.read(ee + 81);
  if (g == 1)
    gainOffset = gO;
  else
    gainOffset = -gO;    
  delay(10);
// ----------------------------------------------------------------------------------------------------------
  for (int i=0; i<buttons; i++) {
    pinMode(ledPin[i],OUTPUT);  
    button[i].debounceTime   = 7;   // Debounce timer in ms
    button[i].multiclickTime = 250;  // Time limit for multi clicks
    button[i].longClickTime  = longClickTime_default; // Time until long clicks register
  }
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
// -------------------------------------------------------- INIT: WAIT FOR BACKUP/RESTORE CUE & BLINK BUTTONS
  for (int i=0; i<2; i++) {
    digitalWrite(ledPin[0], HIGH);
    digitalWrite(ledPin[1], HIGH);
    digitalWrite(ledPin[2], HIGH);
    delay (172);
    digitalWrite(ledPin[0], LOW);
    digitalWrite(ledPin[1], LOW);
    digitalWrite(ledPin[2], LOW);
    delay (172);
  }
  if (!digitalRead(buttonPin1))
    backupPresets();
  if (!digitalRead(buttonPin2) && digitalRead(buttonPin3))
    resetOffset();
  if (!digitalRead(buttonPin2) && !digitalRead(buttonPin3))
    restoreBackup();
  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);
// --------------------------------------------------------------------- SERVOS RUSHES TO MIDDLE POSITION FIX
  volume = EEPROM.read(ee + 0);    // 400++ for last position --> eeprom killer
  shape  = EEPROM.read(ee + 1);
  gain =   EEPROM.read(ee + 2);
  lastVolume = (((volume + masterOffset) * 10) + 600);
  lastShape = (((shape + shapeOffset) * 10) + 600);
  lastGain = (((gain + gainOffset) * 10) + 600);
  lastVolume = constrain(lastVolume, 600, 2180);  // limit to range of servos in ms
  lastShape = constrain(lastShape, 600, 2180);
  lastGain = constrain(lastGain, 600, 2180);
// ------------------------------------------------------------------------------------ SERVO ATTACH POSITION
  VolumeServo.writeMicroseconds(lastVolume);      
  ShapeServo.writeMicroseconds(lastShape);       
  GainServo.writeMicroseconds(lastGain);    
  VolumeServo.attach(VolumePin);
  ShapeServo.attach(ShapePin);
  GainServo.attach(GainPin);
  delay(DELAY);
  VolumeServo.slowmove(90,volumeSpeed);  // middle position
  ShapeServo.slowmove(shapeMax,shapeSpeed);  // end position
  GainServo.slowmove(gainMin,gainSpeed);     // start position
}
// ----------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------FUNCTIONS
// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------------ BACKUP PRESETS TO EEPROM (920++) AND SERIAL.PRINTLN
int backupPresets() {
  pressed[0] = 0;
  editState = 997;
  Serial.println("Service Menu");
  blinkButtonOnce(ledPin[0]);
  blinkButtonOnce(ledPin[1]);
  blinkButtonOnce(ledPin[2]);
  Serial.println("Listing old backup (EEPROM 920 - 1007):");
  Serial.print("[");
  for (int a=920; a<1008; a++) {
    int zz = EEPROM.read(a);
    delay(5);
    Serial.print(zz);
    Serial.print(", ");
  }
  Serial.println("]");
  Serial.println();
  Serial.print("Creating backup (EEPROM ");
  Serial.print(ee); 
  Serial.print(" - ");
  Serial.print(ee + adr -1);
  Serial.print (" to 920 - 1007):");
  Serial.println();
  Serial.print("{");
  int c = 0;
  for (int a=ee; a<ee+adr; a++) {
    int zz = EEPROM.read(a);
    delay(5);
    Serial.print(zz);
    Serial.print(", ");
    EEPROM.write((920+c), zz);
    c++;
  }
  Serial.println("}");
  editState = 0;
}
// ------------------------------------------------------------------------------------------- RESTORE BACKUP
int restoreBackup() {
  pressed[1] = 0;
  pressed[2] = 0;
  editState = 997;
  Serial.print("Restoring backup (to EEPROM ");
  Serial.print(ee); 
  Serial.print(" - ");
  Serial.print(ee + adr -1);
  Serial.print (")");
  Serial.println();
  blinkButtonOnce(ledPin[2]);
  blinkButtonOnce(ledPin[1]);
  blinkButtonOnce(ledPin[0]);
  int c = 0;
  for (int a=920; a<1008; a++) {
    int zz = EEPROM.read(a);
    delay(5);
    EEPROM.write((ee+c), zz);
    c++;
  }
  editState = 0;
}
// --------------------------------------------------------------------------------------------- RESET OFFSET
int resetOffset() { 
  pressed[1] = 0;
  editState = 996;
  blinkButtonOnce(ledPin[1]);
  masterOffset=0;
  shapeOffset=0;
  gainOffset=0;
  delay (1000);
  editState = 0;
}
// ------------------------------------------------------------------ CHANGE ADRESSES TO STORE DATA ON EEPROM
int changeEEPROMadress() {
  pressed[0] = 0;
  pressed[1] = 0;
  pressed[2] = 0;
  editState = 997;
  int ee_old = ee;
  eepromLevelling++;
  if (eepromLevelling > 10) 
    eepromLevelling = 0;
  ee = eepromLevelling * adr;
  int c = 0;
  for (int a=ee_old; a<ee_old+adr; a++) {
    int zz = EEPROM.read(a);
    delay(5);
    EEPROM.write((ee+c), zz);
    c++;
  }
  EEPROM.write(1020, eepromLevelling); 
  editState = 0;
  ledStates();
  setLongClickTime(longClickTime_default);
}
// ------------------------------------------------------------------------------------------------- FREEMODE
int freemode() {
  ledsOff();
  blinkButtonOnce(B_PIN);
  delay(200);
  blinkButtonOnce(B_PIN);
  editState = 998;   // avoid tripple click to exit is also triggered
  setLongClickTime(271);
}
// ------------------------------------------------------------------------------------------------ METRONOME
int metronome() {
  oldEditState = editState;
  editState = 998;
  ledsOff();
  int counter = 0;
  while(1){
    x++;
    if (x > 7) {
     trigger = true;
     x = 0; 
    } else
      trigger = false;    
    counter++;
    delay (1);
    if (counter < 101)
      analogWrite(G_PIN, 72);
    if (counter > 100)
      analogWrite(G_PIN, 0);
    if (counter > tempo && counter < (tempo+100))
      digitalWrite(ledPin[0], HIGH);
    if (counter > (tempo+100))
      digitalWrite(ledPin[0], LOW);
    if (counter > ((tempo*2)+100) && counter < ((tempo*2)+200))
      digitalWrite(ledPin[1], HIGH);
    if (counter > (tempo*2)+200)
      digitalWrite(ledPin[1], LOW);
    if (counter > (tempo*3)+200 && counter < ((tempo*3)+300))
      digitalWrite(ledPin[2], HIGH);
    if (counter > (tempo*3)+300)
      digitalWrite(ledPin[2], LOW);
    if (counter > (tempo*4)+300)
      counter = 0;
    if (!digitalRead(buttonPin1)) {
      break;
    }
    if (!digitalRead(buttonPin2)) {
      if (trigger == true)
        tempo++;
    }
    if (!digitalRead(buttonPin3)) {
      if (trigger == true) {
        tempo--;
        if (tempo < 0)
          tempo = 0;
      }
    }    
  }
  editState = oldEditState;
  ledStates();
  delay(721);
}
// ----------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------- MOVE ALL SERVOS INTO POSITION
int moveServos() {
  int v = constrain((volume + masterOffset), volumeMin, volumeMax);
  v = constrain((v + (volumeBoost * volumeBoostCount)), volumeMin, volumeMax);
  VolumeServo.slowmove(v,volumeSpeed);
  int s = shape + shapeOffset;
  s = constrain(s, shapeMin, shapeMax);
  ShapeServo.slowmove(s,shapeSpeed);
  int g = constrain((gain + gainOffset), gainMin, gainMax);
  g = constrain((g  + (gainBoost * gainBoostCount)), gainMin, gainMax);
  GainServo.slowmove(g,gainSpeed);
}  
// --------------------------------------------------------------- ACCELERATION FOR SERVO SPEEDS IN EDIT MODE
int ease(int which, int whichmin, int whichmax, int a) {
  float d = a;
  if (!digitalRead(buttonPin2) || !digitalRead(buttonPin3)) { 
    y = y - x;
    d = a - sqrt(x)/2;
    d = constrain (d, 1, a);
    if (y < 0) {
      if (!digitalRead(buttonPin2)){
        which--;
        delay(d);
      }
      if (!digitalRead(buttonPin3)) {
        which++;
        delay(d);
      }
      x = x + sqrt(x)/2;
      y = a;
    } 
  } else {
    y = a;
    x = 1;
  }
  which = constrain(which, whichmin, whichmax);
  return(which);  
}
// ----------------------------------------------------------------------------------------- BLINK BUTTON LED
int blinkButtonOnce(int pin) {
  if (digitalRead(pin)) {
    digitalWrite(pin, LOW);
    delay (100);
    digitalWrite(pin, HIGH);
    delay (100);
  } else {
    digitalWrite(pin, HIGH);
    delay (100);
    digitalWrite(pin, LOW);
    delay (100);
  }         
}
// ------------------------------------------------------------------------------------ BLINK ALL BUTTON LEDs
int blinkButtons(boolean Pin0,boolean Pin1,boolean Pin2) {
  blinker++;
  if (blinker > 80)
    blinker = 0;
  if (blinker > 40 && blinker < 80) {
    if (Pin0 == 1)  
      digitalWrite(ledPin[0], LOW);
    if (Pin1 == 1)  
      digitalWrite(ledPin[1], LOW);
    if (Pin2 == 1)  
      digitalWrite(ledPin[2], LOW); 
  } else {
    if (Pin0 == 1)  
      digitalWrite(ledPin[0], HIGH);
    if (Pin1 == 1)  
      digitalWrite(ledPin[1], HIGH);
    if (Pin2 == 1)  
      digitalWrite(ledPin[2], HIGH); 
  }    
}
// --------------------------------------------------------------------------------------------- ALL LEDs OFF
int ledsOff() {
  digitalWrite(ledPin[0], LOW);
  digitalWrite(ledPin[1], LOW);
  digitalWrite(ledPin[2], LOW); 
  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);
}  
// --------------------------------------------------------------------------------- SET VALUE-INDICATOR LEDs
int indicator(int which, int marker1, int marker2, int marker3) {
  digitalWrite(ledPin[0], LOW);
  digitalWrite(ledPin[1], LOW);
  digitalWrite(ledPin[2], LOW); 
  if (which > marker1)
    digitalWrite(ledPin[0], HIGH);
  else 
    digitalWrite(ledPin[0], LOW);
  if (which > marker2)
    digitalWrite(ledPin[1], HIGH);
  else 
    digitalWrite(ledPin[1], LOW);
  if (which > marker3)
    digitalWrite(ledPin[2], HIGH);
  else 
    digitalWrite(ledPin[2], LOW);
}

int offsetIndicator(int which, int ledpin) {
  if (which < 0) {
    digitalWrite(ledPin[0], HIGH);
    digitalWrite(ledPin[1], HIGH);
    digitalWrite(ledPin[2], LOW);
    analogWrite(ledpin, which * -2.833333);
  }
  if (which == 0) {
    digitalWrite(ledPin[0], HIGH);
    digitalWrite(ledPin[1], HIGH);
    digitalWrite(ledPin[2], HIGH);
    blinker++;
    if (blinker > 80)
      blinker = 0;
    if (blinker > 40 && blinker < 80)
      analogWrite(ledpin, 0); 
    else 
      analogWrite(ledpin, 50);
  }
  if (which > 0) {
    digitalWrite(ledPin[0], LOW);
    digitalWrite(ledPin[1], HIGH);
    digitalWrite(ledPin[2], HIGH);
    analogWrite(ledpin, which * 2.833333);
  }  
}
// ---------------------------------------------------------------- SET BUTTON_longClickTime TO DEFAULT VALUE
int setLongClickTime(int value) {
  button[0].longClickTime = value;
  button[1].longClickTime = value;
  button[2].longClickTime = value;
}
// ---------------------------------------------------- CHECK WHAT STATE IS SELECTED AND LIGHT THE RIGHT LEDs
int ledStates() {
  delay (2);
  digitalWrite(ledPin[0], LOW);
  digitalWrite(ledPin[1], LOW);
  digitalWrite(ledPin[2], LOW);
  switch (state) {
    case 1:
      if (bank == 1) 
        digitalWrite(ledPin[0], HIGH);
      else
        digitalWrite(ledPin[0], LOW);
      break;
    case 2:
      if (bank == 1) 
        digitalWrite(ledPin[1], HIGH);
      else
        digitalWrite(ledPin[1], LOW);
      break;
    case 3:
      if (bank == 1) 
        digitalWrite(ledPin[2], HIGH);
      else
        digitalWrite(ledPin[2], LOW);
      break;
    case 4:
      if (bank == 2) 
        digitalWrite(ledPin[0], HIGH);
      else
        digitalWrite(ledPin[0], LOW);
      break;
    case 5:
      if (bank == 2) 
        digitalWrite(ledPin[1], HIGH);
      else
        digitalWrite(ledPin[1], LOW);
      break;
    case 6:
      if (bank == 2) 
        digitalWrite(ledPin[2], HIGH);
      else
        digitalWrite(ledPin[2], LOW);
      break;
    case 7:
      if (bank == 3) 
        digitalWrite(ledPin[0], HIGH);
      else
        digitalWrite(ledPin[0], LOW);
      break;
    case 8:
      if (bank == 3) 
        digitalWrite(ledPin[1], HIGH);
      else
        digitalWrite(ledPin[1], LOW);
      break;
    case 9:
      if (bank == 3) 
        digitalWrite(ledPin[2], HIGH);
      else
        digitalWrite(ledPin[2], LOW);
      break;
    case 10:
      if (bank == 4) 
        digitalWrite(ledPin[0], HIGH);
      else
        digitalWrite(ledPin[0], LOW);
      break;
    case 11:
      if (bank == 4) 
        digitalWrite(ledPin[1], HIGH);
      else
        digitalWrite(ledPin[1], LOW);
      break;
    case 12:
      if (bank == 4) 
        digitalWrite(ledPin[2], HIGH);
      else
        digitalWrite(ledPin[2], LOW);
      break;
  }  
  switch (bank) {
    case 1: r = 0;   g = 0;   b = 0;   break;
    case 2: r = 0;   g = 10;  b = 0;   break;
    case 3: r = 0;   g = 0;   b = 20;  break;
    case 4: r = 5;  g = 5;  b = 5;  break;
  }
  analogWrite(R_PIN, r);
  analogWrite(G_PIN, g);
  analogWrite(B_PIN, b);
}
// ----------------------------------------------------------------------- LOAD PRESET IF BUTTON1 WAS PRESSED
int loadBtn1() {
  switch (bank) {
    case 1:
      volume = EEPROM.read(ee + 0);
      shape  = EEPROM.read(ee + 1);
      gain = EEPROM.read(ee + 2);
      volumeSpeed = EEPROM.read(ee + 36);
      shapeSpeed = EEPROM.read(ee + 37);
      gainSpeed = EEPROM.read(ee + 38);
      state = 1;
      break;
    case 2:
      volume = EEPROM.read(ee + 9);
      shape  = EEPROM.read(ee + 10);
      gain = EEPROM.read(ee + 11);
      volumeSpeed = EEPROM.read(ee + 45);
      shapeSpeed = EEPROM.read(ee + 46);
      gainSpeed = EEPROM.read(ee + 47);
      state = 4;
      break;
    case 3:
      volume = EEPROM.read(ee + 18);
      shape  = EEPROM.read(ee + 19);
      gain = EEPROM.read(ee + 20);
      volumeSpeed = EEPROM.read(ee + 54);
      shapeSpeed = EEPROM.read(ee + 55);
      gainSpeed = EEPROM.read(ee + 56);
      state = 7;
      break;
    case 4:
      volume = EEPROM.read(ee + 27);
      shape  = EEPROM.read(ee + 28);
      gain = EEPROM.read(ee + 29);
      volumeSpeed = EEPROM.read(ee + 63);
      shapeSpeed = EEPROM.read(ee + 64);
      gainSpeed = EEPROM.read(ee + 65);
      state = 10;
      break;
  } 
}
// ----------------------------------------------------------------------- LOAD PRESET IF BUTTON2 WAS PRESSED
int loadBtn2() {  
  switch (bank) {
    case 1:
      volume = EEPROM.read(ee + 3);
      shape  = EEPROM.read(ee + 4);
      gain = EEPROM.read(ee + 5);
      volumeSpeed = EEPROM.read(ee + 39);
      shapeSpeed = EEPROM.read(ee + 40);
      gainSpeed = EEPROM.read(ee + 41);
      state = 2;
      break;
    case 2:
      volume = EEPROM.read(ee + 12);
      shape  = EEPROM.read(ee + 13);
      gain = EEPROM.read(ee + 14);
      volumeSpeed = EEPROM.read(ee + 48);
      shapeSpeed = EEPROM.read(ee + 49);
      gainSpeed = EEPROM.read(ee + 50);
      state = 5;
      break;
    case 3:
      volume = EEPROM.read(ee + 21);
      shape  = EEPROM.read(ee + 22);
      gain = EEPROM.read(ee + 23);
      volumeSpeed = EEPROM.read(ee + 57);
      shapeSpeed = EEPROM.read(ee + 58);
      gainSpeed = EEPROM.read(ee + 59);
      state = 8;
      break;
    case 4:
      volume = EEPROM.read(ee + 30);
      shape  = EEPROM.read(ee + 31);
      gain = EEPROM.read(ee + 32);
      volumeSpeed = EEPROM.read(ee + 66);
      shapeSpeed = EEPROM.read(ee + 67);
      gainSpeed = EEPROM.read(ee + 68);
      state = 11;
      break;
  }
}
// ----------------------------------------------------------------------- LOAD PRESET IF BUTTON3 WAS PRESSED
int loadBtn3() {        
  switch (bank) {
    case 1:
      volume = EEPROM.read(ee + 6);
      shape  = EEPROM.read(ee + 7);
      gain = EEPROM.read(ee + 8);
      volumeSpeed = EEPROM.read(ee + 42);
      shapeSpeed = EEPROM.read(ee + 43);
      gainSpeed = EEPROM.read(ee + 44);
      state = 3;
      break;
    case 2:
      volume = EEPROM.read(ee + 15);
      shape  = EEPROM.read(ee + 16);
      gain = EEPROM.read(ee + 17);
      volumeSpeed = EEPROM.read(ee + 51);
      shapeSpeed = EEPROM.read(ee + 52);
      gainSpeed = EEPROM.read(ee + 53);
      state = 6;
      break;
    case 3:
      volume = EEPROM.read(ee + 24);
      shape  = EEPROM.read(ee + 25);
      gain = EEPROM.read(ee + 26);
      volumeSpeed = EEPROM.read(ee + 60);
      shapeSpeed = EEPROM.read(ee + 61);
      gainSpeed = EEPROM.read(ee + 62);
      state = 9;
      break;
    case 4:
      volume = EEPROM.read(ee + 33);
      shape  = EEPROM.read(ee + 34);
      gain = EEPROM.read(ee + 35);
      volumeSpeed = EEPROM.read(ee + 69);
      shapeSpeed = EEPROM.read(ee + 70);
      gainSpeed = EEPROM.read(ee + 71);
      state = 12;
      break;
  }
}
// ----------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------- EDIT PRESET
int edit() {
  editState = 1;
  volumeBoostCount = 0;
  gainBoostCount = 0;
  setLongClickTime(500);
  analogWrite(R_PIN, volume - volumeMin + 1);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0); 
  indicator(volume, volumeIndicator1, volumeIndicator2, volumeIndicator3);
/*
Serial.print("--- Editing ");
Serial.print(state);
Serial.println(" ---");
Serial.print("Volume :");
*/
}
// ---------------------------------------------------------------------------------------------- COPY PRESET
int copyPreset() {
  editState = 900;
  timer = 0;
  blinker = 0;
  while (1) {
   if (!digitalRead(buttonPin1) || !digitalRead(buttonPin2) || !digitalRead(buttonPin3))
    break;
   timer++;
    if (timer == 400) {
      state = oldState;
      setLongClickTime(longClickTime_default);
      editState = 0;
      ledStates();
      timer = 0;
      break;
    }
    blinkButtons(1,1,1);
    delay(DELAY*2);    
  }
}
// ------------------------------------------------------------------------------------------- RESTORE PRESET
int restorePreset() {
  switch (lastBtn) {
    case 1:
      loadBtn1();
      break;
    case 2:
      loadBtn2();
      break;
    case 3:
      loadBtn3();
      break;
  }    
}
// ------------------------------------------------------------------------------------ SAVE PRESET TO EEPROM
int savePreset() {
  switch (state) {
    case 1:
      EEPROM.write((ee + 0), volume);
      EEPROM.write((ee + 1), shape);
      EEPROM.write((ee + 2), gain);
      EEPROM.write((ee + 36), volumeSpeed);
      EEPROM.write((ee + 37), shapeSpeed);
      EEPROM.write((ee + 38), gainSpeed);
      break;
    case 2:
      EEPROM.write((ee + 3), volume);
      EEPROM.write((ee + 4), shape);
      EEPROM.write((ee + 5), gain);
      EEPROM.write((ee + 39), volumeSpeed);
      EEPROM.write((ee + 40), shapeSpeed);
      EEPROM.write((ee + 41), gainSpeed);
      break;
    case 3:
      EEPROM.write((ee + 6), volume);
      EEPROM.write((ee + 7), shape);
      EEPROM.write((ee + 8), gain);
      EEPROM.write((ee + 42), volumeSpeed);
      EEPROM.write((ee + 43), shapeSpeed);
      EEPROM.write((ee + 44), gainSpeed);
      break;
    case 4:
      EEPROM.write((ee + 9), volume);
      EEPROM.write((ee + 10), shape);
      EEPROM.write((ee + 11), gain);
      EEPROM.write((ee + 45), volumeSpeed);
      EEPROM.write((ee + 46), shapeSpeed);
      EEPROM.write((ee + 47), gainSpeed);
      break;
    case 5:
      EEPROM.write((ee + 12), volume);
      EEPROM.write((ee + 13), shape);
      EEPROM.write((ee + 14), gain);
      EEPROM.write((ee + 48), volumeSpeed);
      EEPROM.write((ee + 49), shapeSpeed);
      EEPROM.write((ee + 50), gainSpeed);
      break;
    case 6:
      EEPROM.write((ee + 15), volume);
      EEPROM.write((ee + 16), shape);
      EEPROM.write((ee + 17), gain);
      EEPROM.write((ee + 51), volumeSpeed);
      EEPROM.write((ee + 52), shapeSpeed);
      EEPROM.write((ee + 53), gainSpeed);
      break;
    case 7:
      EEPROM.write((ee + 18), volume);
      EEPROM.write((ee + 19), shape);
      EEPROM.write((ee + 20), gain);
      EEPROM.write((ee + 54), volumeSpeed);
      EEPROM.write((ee + 55), shapeSpeed);
      EEPROM.write((ee + 56), gainSpeed);
      break;
    case 8:
      EEPROM.write((ee + 21), volume);
      EEPROM.write((ee + 22), shape);
      EEPROM.write((ee + 23), gain);
      EEPROM.write((ee + 57), volumeSpeed);
      EEPROM.write((ee + 58), shapeSpeed);
      EEPROM.write((ee + 59), gainSpeed);
      break;
    case 9:
      EEPROM.write((ee + 24), volume);
      EEPROM.write((ee + 25), shape);
      EEPROM.write((ee + 26), gain);
      EEPROM.write((ee + 60), volumeSpeed);
      EEPROM.write((ee + 61), shapeSpeed);
      EEPROM.write((ee + 62), gainSpeed);
      break;
    case 10:
      EEPROM.write((ee + 27), volume);
      EEPROM.write((ee + 28), shape);
      EEPROM.write((ee + 29), gain);
      EEPROM.write((ee + 63), volumeSpeed);
      EEPROM.write((ee + 64), shapeSpeed);
      EEPROM.write((ee + 65), gainSpeed);
      break;
    case 11:
      EEPROM.write((ee + 30), volume);
      EEPROM.write((ee + 31), shape);
      EEPROM.write((ee + 32), gain);
      EEPROM.write((ee + 66), volumeSpeed);
      EEPROM.write((ee + 67), shapeSpeed);
      EEPROM.write((ee + 68), gainSpeed);
      break;
    case 12:
      EEPROM.write((ee + 33), volume);
      EEPROM.write((ee + 34), shape);
      EEPROM.write((ee + 35), gain);
      EEPROM.write((ee + 69), volumeSpeed);
      EEPROM.write((ee + 70), shapeSpeed);
      EEPROM.write((ee + 71), gainSpeed);
      break;
  }
}
// --------------------------------------------------------------------------------------------- BOOST VOLUME
int boostVolume() {
  int v;
  ledsOff();
  delay(10);
  v = constrain((volume + masterOffset), volumeMin, volumeMax);
  v = constrain((v + (volumeBoost * volumeBoostCount)), volumeMin, volumeMax);
  VolumeServo.slowmove(v,speedVolumeboost);
  blinker = 0;
  for (int i=0; i<10; i++) {
    blinkButtons(1,1,1);
    delay (2);
  }  
  ledStates();
  editState = 10;
}
// ----------------------------------------------------------------------------------------------- BOOST GAIN
int boostGain() {
  int g;
  ledsOff();
  delay(10);
  g = constrain((gain + gainOffset), gainMin, gainMax);
  g = constrain((g + (gainBoost * gainBoostCount)), gainMin, gainMax);
  GainServo.slowmove(g,speedGainboost);
  blinker = 0;
  for (int i=0; i<10; i++) {
    blinkButtons(1,1,1);
    delay (2);
  }  
  ledStates();
  editState = 10;
}
// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ VOID LOOP
// ----------------------------------------------------------------------------------------------------------
void loop() {
// check button events
  for (int i=0; i<buttons; i++)
  {
    button[i].Update();
    if (button[i].clicks != 0) pressed[i] = button[i].clicks;
// -------------------------------------------------------------------------------------- BUTTON1 SHORT CLICK
    if(pressed[0] == 1) {
      if (editState > 0 && editState < 777 && editState != 10)
        editState++;
// ---------------------------------------------------------------------------------------------- NORMAL MODE
      if (editState == 0 || editState == 10) {
        editState = 0;
        volumeBoostCount = 0;
        gainBoostCount = 0;
        ledStates();
        digitalWrite(ledPin[0], HIGH);
        digitalWrite(ledPin[1], LOW);
        digitalWrite(ledPin[2], LOW);
        loadBtn1();
        moveServos();
      }
  /*
  Serial.print("----- ");
  Serial.print(state);
  Serial.println(" -----");
  Serial.print("Volume :");
  Serial.println(volume);
  Serial.print("Shape :");
  Serial.println(shape);
  Serial.print("Gain :");
  Serial.println(gain);
  Serial.print("Volume Speed :");
  Serial.println(volumeSpeed);
  Serial.print("Shape Speed :");
  Serial.println(shapeSpeed);
  Serial.print("Gain Speed :");
  Serial.println(gainSpeed);
  */
// ------------------------------------------------------------------------------------------------ EDIT MODE
      if (editState == 2) {
        analogWrite(R_PIN, 0);
        analogWrite(G_PIN, shape - shapeMin + 1);
        indicator(shape, shapeIndicator1, shapeIndicator2, shapeIndicator3);
// Serial.print("Shape :");
      }
      if (editState == 3) {
        analogWrite(G_PIN, 0);
        analogWrite(B_PIN, gain - gainMin + 1);
        indicator(gain, gainIndicator1, gainIndicator2, gainIndicator3);
// Serial.println("Gain :");
      }
      if (editState == 4) {
        analogWrite(R_PIN, volumeSpeed);
        analogWrite(G_PIN, 0);
        analogWrite(B_PIN, volumeSpeed);
        indicator(volumeSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
// Serial.print("Volume Speed :");
      }
      if (editState == 5) {
// Serial.println("Shape Speed :");
        analogWrite(R_PIN, shapeSpeed/3);
        analogWrite(G_PIN, shapeSpeed);
        analogWrite(B_PIN, 0);
        indicator(shapeSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
        timer = 0;
        while(1){
          pressed[0] = 0;
          timer++;
          delay(10);
          if (!digitalRead(buttonPin2) || !digitalRead(buttonPin3))
            break;
          button[0].Update();
          if (button[0].clicks != 0) pressed[0] = button[0].clicks;
          if (pressed[0] == 1) {
            editState = 6;
            pressed[0] = 0;
            break;
          }
          if (pressed[0] == -1) {
            editState = 4;
            pressed[0] = 0;
            break;
          }
          if (timer > canelDelay*0.80) {
            digitalWrite(ledPin[0], HIGH);         
            digitalWrite(ledPin[1], LOW);         
            digitalWrite(ledPin[2], HIGH);         
          }
          if (timer > canelDelay*0.825) {
            digitalWrite(ledPin[0], LOW);         
            digitalWrite(ledPin[1], HIGH);         
            digitalWrite(ledPin[2], LOW);         
          }
          if (timer > canelDelay*0.85) {
            digitalWrite(ledPin[0], HIGH);         
            digitalWrite(ledPin[1], LOW);         
            digitalWrite(ledPin[2], HIGH);         
          }
          if (timer > canelDelay*0.875) {
            digitalWrite(ledPin[0], LOW);         
            digitalWrite(ledPin[1], LOW);         
            digitalWrite(ledPin[2], LOW);         
          }
          if (timer > canelDelay) {
            shapeSpeed = volumeSpeed;
            gainSpeed = volumeSpeed;
            editState = 7;
            break;
          }
          
        }
        timer = 0;
      }
      if (editState == 6) {
        analogWrite(R_PIN, 0);
        analogWrite(G_PIN, gainSpeed);
        analogWrite(B_PIN, gainSpeed);
        indicator(gainSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
// Serial.print("Gain Speed :");
      }
      if (editState == 7) {
        analogWrite(R_PIN, 0);
        analogWrite(G_PIN, 0);
        analogWrite(B_PIN, 0);
        savePreset();
        editState = 0;
        ledStates();
        moveServos();
        setLongClickTime(longClickTime_default);
        pressed [0] = 0;
        // break;
      }
      if (editState == 701) {
        if (masterOffset < 0) {
          EEPROM.write((ee + 76), 0);
          int m = -masterOffset;
          EEPROM.write((ee + 77), m);
        } else {
          EEPROM.write((ee + 76), 1);
          int m = masterOffset;
          EEPROM.write((ee + 77), m);
        }
        ledsOff();
      }
      if (editState == 702) {
        if (shapeOffset < 0) {
          EEPROM.write((ee + 78), 0);
          int s = -shapeOffset;
          EEPROM.write((ee + 79), s);
        } else {
          EEPROM.write((ee + 78), 1);
          int s = shapeOffset;
          EEPROM.write((ee + 79), s);
        }        
        ledsOff();
      }
      if (editState == 703) {
        if (gainOffset < 0) {
          EEPROM.write((ee + 80), 0);
          int g = -gainOffset;
          EEPROM.write((ee + 81), g);
        } else {
          EEPROM.write((ee + 80), 1);
          int g = gainOffset;
          EEPROM.write((ee + 81), g);
        }
        setLongClickTime(longClickTime_default);
        editState = 0;
        ledStates();
        moveServos();
      }
      if (editState == 751) {
        EEPROM.write((ee + 72), volumeBoost);
        ledsOff();
      }
      if (editState == 752) {
        EEPROM.write((ee + 73), speedVolumeboost);
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 761) {
        EEPROM.write((ee + 74), gainBoost);
        ledsOff();
      }
      if (editState == 762) {
        EEPROM.write((ee + 75), speedGainboost);
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 771) {
        longClickTime_default = longClickTimeZ * 10;
        //Decomposition from a long to 4 bytes by using bitshift.
        //One = Most significant -> Four = Least significant byte
        byte four = (longClickTime_default & 0xFF);
        byte three = ((longClickTime_default >> 8) & 0xFF);
        byte two = ((longClickTime_default >> 16) & 0xFF);
        byte one = ((longClickTime_default >> 24) & 0xFF);
        //Write the 4 bytes into the eeprom memory.
        EEPROM.write((ee + 82), four);
        EEPROM.write((ee + 83), three);
        EEPROM.write((ee + 84), two);
        EEPROM.write((ee + 85), one);
        ledsOff();
      }
      if (editState == 772) {
        EEPROM.write((ee + 86), easeFreemode);
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 777) {
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 900) {
        switch (bank) {
          case 1: state = 1;   break;
          case 2: state = 4;   break;
          case 3: state = 7;   break;
          case 4: state = 10;  break;
        }         
        savePreset();
        ledStates();
        moveServos();  
        editState = 0;
        setLongClickTime(longClickTime_default);
      }
      if (editState == 999) {
        if (DirectionVolume == true)
          DirectionVolume = false; // Switch direction
        else
          DirectionVolume = true; // Switch direction
      }
      pressed[0] = 0;
//Serial.println("btn1 click released");
    }
// -------------------------------------------------------------------------------------- BUTTON2 SHORT CLICK
    if(pressed[1] == 1) {
      if (editState == 0 || editState == 10) { 
        editState = 0;
        volumeBoostCount = 0;
        gainBoostCount = 0;
        ledStates();
        digitalWrite(ledPin[0], LOW);
        digitalWrite(ledPin[1], HIGH);
        digitalWrite(ledPin[2], LOW);
        loadBtn2();
        moveServos();
      }
      if (editState == 777) {
        changeEEPROMadress();
      }
      if (editState == 900) {
        switch (bank) {
          case 1: state = 2;    break;
          case 2: state = 5;    break;
          case 3: state = 8;    break;
          case 4: state = 11;   break;
        }         
        savePreset();
        ledStates();
        moveServos();  
        editState = 0;
        setLongClickTime(longClickTime_default);
      }
      if (editState == 999) {
        if (DirectionShape == true)
          DirectionShape = false; // Switch direction
        else
          DirectionShape = true; // Switch direction
      }
      pressed[1] = 0;
//Serial.println("btn2 click released");
    }    
// -------------------------------------------------------------------------------------- BUTTON3 SHORT CLICK
    if(pressed[2] == 1) {
      if (editState == 0 || editState == 10) {
        editState = 0;
        volumeBoostCount = 0;
        gainBoostCount = 0;
        ledStates();
        digitalWrite(ledPin[0], LOW);
        digitalWrite(ledPin[1], LOW);
        digitalWrite(ledPin[2], HIGH);
        loadBtn3();
        moveServos();
      } 
      if (editState == 777) {
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 900) {
        switch (bank) { 
          case 1: state = 3;   break;
          case 2: state = 6;   break;
          case 3: state = 9;   break;
          case 4: state = 12;  break;
        }         
        savePreset();
        ledStates();
        moveServos();  
        editState = 0;
        setLongClickTime(longClickTime_default);
      }
      if (editState == 999) {
        if (DirectionGain == true) 
          DirectionGain = false; // Switch direction
        else
          DirectionGain = true; // Switch direction
      }
      pressed[2] = 0;
//  Serial.println("btn3 click released");
    }
// --------------------------------------------------------------------------------------- BUTTON1 LONG CLICK
    if(pressed[0] == -1) {
      if (editState == 0 || editState == 10)
        edit();
      if (editState > 1 && editState < 777 && editState != 10 && editState != 700 && editState != 750 && editState != 760 && editState != 770) {    //  ----- move back in editmode
        editState--;
        if (editState == 1)
          analogWrite(G_PIN, 0); 
        if (editState == 2) 
          analogWrite(B_PIN, 0); 
        if (editState == 3) {
          analogWrite(R_PIN, 0); 
          analogWrite(G_PIN, 0); 
        }
        if (editState == 4) 
          analogWrite(B_PIN, 0); 
        if (editState == 5) 
          analogWrite(R_PIN, 0);
        if (editState == 700)
          analogWrite(G_PIN, 0); 
        if (editState == 701) 
          analogWrite(B_PIN, 0); 
          
      }
// ------------------------------------------------------------------------------------------- CLICK AND HOLD
      if (editState == 999) {
        y = easeFreemode;
        x = 1;
        float d = easeFreemode;
        while(button[0].depressed == true){
          button[0].Update();
          y = y - x;
          d = easeFreemode - sqrt(x)/2;
          d = constrain (d, 1, easeFreemode);
          if (y < 0) {
            if (DirectionVolume) {
              volume++;
              delay(d);
            } else {
              volume--;
              delay(d);
            }
            x = x + sqrt(x)/3;
            y = easeFreemode;
          } 
          volume = constrain(volume, volumeMin, volumeMax);
   //       Serial.print("volume: ");
   //       Serial.println(volume);
          indicator(volume, volumeIndicator1, volumeIndicator2, volumeIndicator3);
          analogWrite(R_PIN, volume + volumeMin - 1);
          analogWrite(G_PIN, 0);
          analogWrite(B_PIN, 0);
          VolumeServo.slowmove(volume,editSpeed);
          ledsOff();
        }
      }     
      pressed[0] = 0;
    }
// --------------------------------------------------------------------------------------- BUTTON2 LONG CLICK
    if(pressed[1] == -1) {
      if (editState == 0 || editState == 10) {
        volumeBoostCount++;
        boostVolume();
      }
      if (editState == 900) {
        bank--;
        if (bank < 1)
          bank = 4;
        ledStates();
      }
// ------------------------------------------------------------------------------------------- CLICK AND HOLD
      if (editState == 999) {
        y = easeFreemode;
        x = 1;
        float d = easeFreemode;
        while(button[1].depressed == true){
          button[1].Update();
          y = y - x;
          d = (1.5 * easeFreemode) - sqrt(x);
          d = constrain (d, 1, (1.5 * easeFreemode));
          if (y < 0) {
            if (DirectionShape) {
              shape++;
              delay(d);
            } else {
              shape--;
              delay(d);
            }
            x = x + sqrt(x)/3;
            y = easeFreemode;
          } 
          shape = constrain(shape, shapeMin, shapeMax);
  //        Serial.print("shape: ");
  //        Serial.println(shape);
          indicator(shape, shapeIndicator1, shapeIndicator2, shapeIndicator3);
          analogWrite(G_PIN, shape + shapeMin - 1);
          analogWrite(R_PIN, 0);
          analogWrite(B_PIN, 0);
          ShapeServo.slowmove(shape,editSpeed);
          ledsOff();
        }
      }     
      pressed[1] = 0;
    }
// -------------------------------------------------------------------------------- BUTTON2 DOUBLE LONG CLICK
    if(pressed[1] == -2) {
      if (editState == 0 || editState == 10 || editState == 900) {
        bank--;
        if (bank < 1)
          bank = 4;
        ledStates();
      }
      pressed[1] = 0;  
    }    
// --------------------------------------------------------------------------------------- BUTTON3 LONG CLICK
    if(pressed[2] == -1) {
      if (editState == 0 || editState == 10) {
        gainBoostCount++;
        boostGain();
      } 
      if (editState == 900) {
        bank++;
        if (bank > 4)
          bank = 1;
        ledStates();
      } 
// ------------------------------------------------------------------------------------------- CLICK AND HOLD
      if (editState == 999) {
        y = easeFreemode;
        x = 1;
        float d = easeFreemode;
        while(button[2].depressed == true){
          button[2].Update();
          y = y - x;
          d = easeFreemode - sqrt(x)/2;
          d = constrain (d, 1, easeFreemode);
          if (y < 0) {
            if (DirectionGain) {
              gain++;
              delay(d);
            } else {
              gain--;
              delay(d);
            }
            x = x + sqrt(x)/3;
            y = easeFreemode;
          } 
          gain = constrain(gain, gainMin, gainMax);
    //      Serial.print("gain: ");
    //      Serial.println(gain);
          indicator(gain, gainIndicator1, gainIndicator2, gainIndicator3);
          analogWrite(B_PIN, gain + gainMin - 1);
          analogWrite(G_PIN, 0);
          analogWrite(R_PIN, 0);
          GainServo.slowmove(gain,editSpeed);
          ledsOff();
        }
      }     
      pressed[2] = 0;
    }
// -------------------------------------------------------------------------------- BUTTON3 DOUBLE LONG CLICK
    if(pressed[2] == -2) {
      if (editState == 0 || editState == 10 || editState == 900) {
        bank++;
        if (bank > 4)
          bank = 1;
        ledStates();
      }
      pressed[2] = 0;  
    }  
// ------------------------------------------------------------------------------------- BUTTON1 DOUBLE CLICK
    if(pressed[0] == 2) {
      if (editState == 1) {
        editState = 700;
        ledsOff();
      }
      pressed[0] = 0;
    }
// ------------------------------------------------------------------------------------- BUTTON2 DOUBLE CLICK
    if(pressed[1] == 2) {
      if (editState == 1) {
        editState = 750;
        ledsOff();
      }
      pressed[1] = 0;
    }
// ------------------------------------------------------------------------------------- BUTTON3 DOUBLE CLICK
    if(pressed[2] == 2) {
      if (editState == 1) {
        editState = 760;
        ledsOff();
      }
      pressed[2] = 0;
    }
// ------------------------------------------------------------------------------------ BUTTON1 TRIPPLE CLICK
    if(pressed[0] == 3) {
      if (editState == 0 || editState == 10) {
        oldState = state;
        oldEditState = editState;
        lastBtn = 1;
        loadBtn1();
        if (state != oldState) {
          volumeBoostCount = 0;
          gainBoostCount = 0;
        }
        moveServos(); 
        volume = constrain((volume + masterOffset), volumeMin, volumeMax);
        volume = constrain((volume  + (volumeBoost * volumeBoostCount)), volumeMin, volumeMax);
        shape = shape + shapeOffset;
        gain = constrain((gain + gainOffset), gainMin, gainMax);
        gain = constrain((gain + (gainBoost * gainBoostCount)), gainMin, gainMax);
        freemode();
      }
      if (editState >0 && editState < 780) {
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
      }
      if (editState == 900) {
        state = oldState;
        bank = oldBank;
        ledStates();
        editState = 0;
      }
      if (editState == 999) {
        ledsOff();
        volumeBoostCount = 0;
        gainBoostCount = 0;
        editState = 0;
        restorePreset();
        ledStates();
        moveServos();
        setLongClickTime(longClickTime_default);
        DirectionVolume = true;
        DirectionShape = true;
        DirectionGain = true;
      }
      pressed[0] = 0;
    }
// ------------------------------------------------------------------------------------ BUTTON2 TRIPPLE CLICK
    if(pressed[1] == 3) {
      if (editState == 0 || editState == 10) {
        oldState = state;
        oldEditState = editState;
        lastBtn = 2;
        loadBtn2();
        if (state != oldState) {
          volumeBoostCount = 0;
          gainBoostCount = 0;
        }
        moveServos();
        volume = constrain((volume + masterOffset), volumeMin, volumeMax);
        volume = constrain((volume  + (volumeBoost * volumeBoostCount)), volumeMin, volumeMax);
        shape = shape + shapeOffset;
        gain = constrain((gain + gainOffset), gainMin, gainMax);
        gain = constrain((gain + (gainBoost * gainBoostCount)), gainMin, gainMax);
        freemode();            
      }
      if (editState == 1) {
        editState = 770;
        setLongClickTime(longClickTime_default);
        longClickTimeZ = longClickTime_default / 10;
        ledsOff();        
      }
      if (editState > 770 && editState < 774) {
        longClickTime_default = 1720;
        easeFreemode = 35;
        byte four = (longClickTime_default & 0xFF);
        byte three = ((longClickTime_default >> 8) & 0xFF);
        byte two = ((longClickTime_default >> 16) & 0xFF);
        byte one = ((longClickTime_default >> 24) & 0xFF);
        //Write the 4 bytes into the eeprom memory.
        EEPROM.write((ee + 82), four);
        EEPROM.write((ee + 83), three);
        EEPROM.write((ee + 84), two);
        EEPROM.write((ee + 85), one);
        EEPROM.write((ee + 86), easeFreemode);
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);        
      }
      if (editState == 999) {
        ledsOff();
        volume = constrain((volume - masterOffset), volumeMin, volumeMax);
        shape = constrain((shape - shapeOffset), shapeMin, shapeMax);
        gain = constrain((gain - gainOffset), gainMin, gainMax);
        delay(500);
        savePreset();
        editState = 0;
        ledStates();
        setLongClickTime(longClickTime_default);
        volumeBoostCount = 0;
        gainBoostCount = 0;
        moveServos();
        DirectionVolume = true;
        DirectionShape = true;
        DirectionGain = true;
      }
      pressed[1] = 0;
    }

// ------------------------------------------------------------------------------------ BUTTON3 TRIPPLE CLICK
    if(pressed[2] == 3) {
      if (editState == 0 || editState == 10) {
        oldState = state;
        oldEditState = editState;
        lastBtn = 3;
        loadBtn3();
        if (state != oldState) {
          volumeBoostCount = 0;
          gainBoostCount = 0;
        }
        moveServos();
        volume = constrain((volume + masterOffset), volumeMin, volumeMax);
        volume = constrain((volume  + (volumeBoost * volumeBoostCount)), volumeMin, volumeMax);
        shape = shape + shapeOffset;
        gain = constrain((gain + gainOffset), gainMin, gainMax);
        gain = constrain((gain + (gainBoost * gainBoostCount)), gainMin, gainMax);
        freemode();
      }
      if (editState == 1) {
        ledStates();
        oldState = state;
        oldBank = bank;
        copyPreset();
      }
      if (editState == 999) {
        ledsOff();
        if (oldEditState == 10) {
          editState = 10;
        } else {
          volumeBoostCount = 0;
          gainBoostCount = 0;
          editState = 0;
        }
        restorePreset();
        ledStates();
        moveServos();
        setLongClickTime(longClickTime_default);
        DirectionVolume = true;
        DirectionShape = true;
        DirectionGain = true;
      }
      pressed[2] = 0;
    }

// ------------------------------------------------------------------------------- BUTTON3 TRIPPLE LONG CLICK
    if(pressed[2] == -3) {
      if (editState == 0 || editState == 10) { 
        metronome();
        button[2].clicks = 0;  //will run twice without this line... BUG??
      }
      if (editState == 770) { 
        ledsOff();
        delay(DELAY * 10);
        blinkButtonOnce(ledPin[0]);
        blinkButtonOnce(ledPin[1]);
        blinkButtonOnce(ledPin[2]);
        blinkButtonOnce(ledPin[1]);
        blinkButtonOnce(ledPin[0]);
        ledsOff();
        editState = 777;
        button[2].clicks = 0;  //will run twice without this line... BUG??
      }
    pressed[2] = 0;  
    }    
//-------    
  }
// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------- EDIT STATE 1
  if (editState == 1) { 
    volume = ease(volume,volumeMin, volumeMax, 27);   
    indicator(volume, volumeIndicator1, volumeIndicator2, volumeIndicator3);
    analogWrite(R_PIN, volume + volumeMin - 1);
    int v = volume + masterOffset;
    v = constrain(v, volumeMin, volumeMax);
    VolumeServo.slowmove(v,editSpeed);
//Serial.println(volume);
  }
// --------------------------------------------------------------------------------------------- EDIT STATE 2
  if (editState == 2) {
    shape = ease(shape, shapeMin, shapeMax, 27);   
    indicator(shape, shapeIndicator1, shapeIndicator2, shapeIndicator3);
    analogWrite(G_PIN, shape + shapeMin - 1);
    int s = shape + shapeOffset;
    s = constrain(s, shapeMin, shapeMax);
    ShapeServo.slowmove(s,editSpeed);
//Serial.println(shape);
  }
// --------------------------------------------------------------------------------------------- EDIT STATE 3
  if (editState == 3) {
    gain = ease(gain, gainMin, gainMax, 27);   
    indicator(gain, gainIndicator1, gainIndicator2, gainIndicator3);
    analogWrite(B_PIN, gain + gainMin - 1);
    int g = gain + gainOffset;
    g = constrain(g, gainMin, gainMax);
    GainServo.slowmove(g,editSpeed);
//Serial.println(gain);
  }
// --------------------------------------------------------------------------------------------- EDIT STATE 4
  if (editState == 4) {
    volumeSpeed = ease(volumeSpeed, 1, 255, 21);   
    indicator(volumeSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
    analogWrite(R_PIN, volumeSpeed);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, volumeSpeed);
  }
// --------------------------------------------------------------------------------------------- EDIT STATE 5
  if (editState == 5) {
    shapeSpeed = ease(shapeSpeed, 1, 255, 21);   
    indicator(shapeSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
    analogWrite(R_PIN, shapeSpeed/3);
    analogWrite(G_PIN, shapeSpeed);
    analogWrite(B_PIN, 0);
  }
// --------------------------------------------------------------------------------------------- EDIT STATE 6
  if (editState == 6) {
    gainSpeed = ease(gainSpeed, 1, 255, 21);   
    indicator(gainSpeed, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
    analogWrite(R_PIN, 0);
    analogWrite(B_PIN, gainSpeed);
    analogWrite(G_PIN, gainSpeed);
  }
// -------------------------------------------------------------------------------------------- EDIT STATE 10
  if (editState == 10) {
    if (state == 1 || state == 4 || state == 7 || state == 10)
      blinkButtons(1,0,0);
    if (state == 2 || state == 5 || state == 8 || state == 11)
      blinkButtons(0,1,0);
    if (state == 3 || state == 6 || state == 9 || state == 12)
      blinkButtons(0,0,1);
    delay(DELAY);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 700
  if (editState == 700) { 
    masterOffset = ease(masterOffset, -90, 90, 27);   
    offsetIndicator(masterOffset, R_PIN);
    int v = volume + masterOffset;
    v = constrain(v, volumeMin, volumeMax);
    VolumeServo.slowmove(v,editSpeed);
// Serial.println(masterOffset);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 701
  if (editState == 701) { 
    shapeOffset = ease(shapeOffset, -90, 90, 27);   
    offsetIndicator(shapeOffset, G_PIN);
    int s = shape + shapeOffset;
    s = constrain(s, shapeMin, shapeMax);
    ShapeServo.slowmove(s,editSpeed);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 702
  if (editState == 702) { 
    gainOffset = ease(gainOffset, -90, 90, 27);   
    offsetIndicator(gainOffset, B_PIN);
    int g = gain + gainOffset;
    g = constrain(g, gainMin, gainMax);
    GainServo.slowmove(g,editSpeed);
  }
// ---------------------------------------------------------------- SETUP BOOST VALUES - EDIT STATE 750 / 760
// ------------------------------------------------------------------------------------------- EDIT STATE 750
  if (editState == 750) {
    volumeBoost = ease(volumeBoost, 1, 255, 21);   
    indicator(volumeBoost, 5, 15, 30);
    analogWrite(R_PIN, volumeBoost + volumeMin - 1);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 751
  if (editState == 751) {
    speedVolumeboost = ease(speedVolumeboost, 1, 255, 21);   
    indicator(speedVolumeboost, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
    analogWrite(R_PIN, speedVolumeboost);
    analogWrite(G_PIN, speedVolumeboost);
    analogWrite(B_PIN, 0);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 760
  if (editState == 760) {
    gainBoost = ease(gainBoost, 1, 255, 21);   
    indicator(gainBoost, 5, 15, 30);
    analogWrite(R_PIN, 0);
    analogWrite(B_PIN, gainBoost + gainMin - 1);
    analogWrite(G_PIN, 0);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 761
  if (editState == 761) {
    speedGainboost = ease(speedGainboost, 1, 255, 21);   
    indicator(speedGainboost, servoSpeedsIndicator1, servoSpeedsIndicator2, servoSpeedsIndicator3);
    analogWrite(R_PIN, 0);
    analogWrite(B_PIN, speedGainboost);
    analogWrite(G_PIN, speedGainboost);
  }
// --------------------------------------------------------------- SETUP SYSTEM VALUES - EDIT STATE 770 / 771
// ------------------------------------------------------------------------------------------- EDIT STATE 770
  if (editState == 770) {
    longClickTimeZ = ease(longClickTimeZ, 20, 220, 8);   
    indicator(longClickTimeZ, 100, 140, 172);
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, longClickTimeZ);
    analogWrite(B_PIN, longClickTimeZ);
//    Serial.print("longClickTime_default :");
//    Serial.println(longClickTime_default);
  }
// ------------------------------------------------------------------------------------------- EDIT STATE 771
  if (editState == 771) {
    easeFreemode = ease(easeFreemode, 1, 255, 21);   
    indicator(easeFreemode, 15, 35, 70);
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, easeFreemode);
    analogWrite(B_PIN, 0);
//    Serial.print("easeFreemode :");
//    Serial.println(easeFreemode);
  }
// ------------------------------------------------------------------- EDIT STATE 777 - SHIFT EEPROM ADRESSES
  if (editState == 777) {
    blinkButtons(0,1,0);
  }
// ----------------------------------------------------------------------------- EDIT STATE 900 - COPY PRESET
  if (editState == 900) {
    blinkButtons(1,1,1);
    delay(DELAY);
  }
// -------------------------------------------------------------------------------- EDIT STATE 999 - FREEMODE
  if (editState == 998)
    editState++;   // avoid tripple click to exit is also triggered
  if (editState == 999) {
    if (DirectionVolume == true)
      digitalWrite(ledPin[0], HIGH); 
    else  
      digitalWrite(ledPin[0], LOW); 
    if (DirectionShape == true)
      digitalWrite(ledPin[1], HIGH);   
    else  
      digitalWrite(ledPin[1], LOW); 
    if (DirectionGain == true)
      digitalWrite(ledPin[2], HIGH);   
    else  
      digitalWrite(ledPin[2], LOW); 
  }
// ----------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------ ENTER FREEMODE OR METRONOME
/*   if (button[0].depressed == true && button[2].depressed == false){  // press left button first
     if (!digitalRead(buttonPin3))                                    // then right button
       freemode();
   }
*/
// ----------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------- NO LAG EVENT
// ------------------- ALLOW IMEDIATE BUTTON RESPONSE WHILE SWITCHING PRESETS FROM BTN3 TO BTN2 IN BANK 3 + 4 
  if (!digitalRead(buttonPin2)) {
    if (editState == 0 && bank > 2) {
      if (state == 9 || state == 12) {
        digitalWrite(ledPin[0], LOW);
        digitalWrite(ledPin[1], HIGH);
        digitalWrite(ledPin[2], LOW);
        loadBtn2();
        moveServos();
      }
    }
  }
// ------------------- ALLOW IMEDIATE BUTTON RESPONSE WHILE SWITCHING PRESETS FROM BTN2 TO BTN3 IN BANK 3 + 4 
  if (!digitalRead(buttonPin3)) {
    if (editState == 0 && bank > 2) {
      if (state == 8 || state == 11) {
        digitalWrite(ledPin[0], LOW);
        digitalWrite(ledPin[1], LOW);
        digitalWrite(ledPin[2], HIGH);
        loadBtn3();
        moveServos();
      }
    }
  }
// ----------------------------------------------- END OF LOOP  
  delay(DELAY);
//  Serial.println(); 
}
