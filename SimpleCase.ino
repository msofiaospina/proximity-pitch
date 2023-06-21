// Include the library:
#include <SharpIR.h> /*SHARP GP2Y0A21YK0F IR distance sensor with Arduino and SharpIR library example code. More info: https://www.makerguides.com */
#include <MIDIUSB.h> //MIDIUSB has a library called "PitchToNote.h" which converts notes like C3,D3,C5 etc to intergers
#include <Keyboard.h> //For assigning buttons to a specific key strokes
#include <Adafruit_NeoPixel.h> //Light control https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels?view=all#powering-neopixels

// Define model and input pin:
#define IRPin A0 //Distance sensor
#define model 1080
#define LED_PIN    7 //Neo Pixel
#define LED_COUNT 25 //Strand length
int LEDNum  = 24;

// Create variables:
// buttons():
int inst1Pin = 2;
int inst2Pin = 3;
int inst3Pin = 4;
int buttonPressTime = 0;
int buttonTimeout = 150;
int buttonPressed;
int oldButton;
int change;
//determineNote() & turnOn() & turnOff():
int distance_cm; //Distance sensor response
int noteToBePlayed = 0; //Distance translated to a midi note
int oldNote = 1;
int upperNote = 100; //PitchToNote.h
int lowerNote = 30; //PitchToNote.h
int listLength = 30; //array length
int listOfNotesToTurnOff[30]; //Delayed note shut off. This array is set to 30 (listLength) to be able to clear a note and meet delay without filling up
unsigned long timesToTurnOffNotes[30]; //Delayed note shut off
unsigned long delayNote;
int noteDuration = 500;
int delayTime = 100; //milliseconds
int k = 0;
int j;
int maxPlayableDistance = 60; //cm
int minPlayableDistance = 10; //cm
int assingDistanceToPentatonic; //index
//lights()
int oldLEDNum = 0;
bool flag = false;
unsigned long sensorTimeOut;
int interval = 5000; //milliseconds
//autoPlay()
unsigned long checkAutoPlay;
int timeForAutoPlay = 10000;
bool flag1 = false;
unsigned long someoneIsPlaying;
int timeSinceSomeoneIsPlaying = 5000;
int twinkle[42] = {72,72,79,79,81,81,79,65,65,52,52,38,38,24,79,79,65,65,52,52,38,79,79,65,65,52,52,38,72,72,79,79,81,81,79,65,65,52,52,38,38,24};
int pentatonic[24] = {31,33,35,38,40,43,33,35,37,40,42,45,35,37,39,42,44,47,36,38,40,43,45,48};


//Create a new instance of the SharpIR class:
SharpIR mySensor = SharpIR(IRPin, model);

//Declare NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(inst1Pin, INPUT_PULLUP);
  pinMode(inst2Pin, INPUT_PULLUP);
  pinMode(inst3Pin, INPUT_PULLUP);
  Serial.begin(9600);
  Keyboard.begin();

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void buttons() {
  if (digitalRead(inst1Pin)==LOW){
  change = buttonPressed - oldButton;
    if(change > 0){
      for (int i = 1; i <= change; i++){
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.releaseAll();
      }
    }
    if(change < 0){
      for (int i = 1; i <= abs(change); i++){
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.releaseAll();
      }
    }
    oldButton = buttonPressed;
  }
  if (digitalRead(inst2Pin)==LOW){
  buttonPressed = 1;
  change = buttonPressed - oldButton;
    if(change > 0){
      for (int i = 1; i <= change; i++){
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.releaseAll();
      }
    }
    if(change < 0){
      for (int i = 1; i <= abs(change); i++){
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.releaseAll();
      }
    }
    oldButton = buttonPressed;
  }
  if (digitalRead(inst3Pin)==LOW){
  buttonPressed = 2;
  change = buttonPressed - oldButton;
    if(change > 0){
      for (int i = 1; i <= change; i++){
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.releaseAll();
      }
    }
    if(change < 0){
      for (int i = 1; i <= abs(change); i++){
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.releaseAll();
      }
    }
    oldButton = buttonPressed;
  }
}

void MIDIButtons() {
  if (digitalRead(inst1Pin)==LOW){
    Serial.println("made it");
    controlChange(0, 64, 127);//  (channel, CC number,  CC value)
    MidiUSB.flush();
    //delay();
    //controlChange(0, 64, 0);//  (channel, CC number,  CC value)
    //MidiUSB.flush();
  }
  else {
    Serial.println("off");
    controlChange(0, 64, 0);//  (channel, CC number,  CC value)
    MidiUSB.flush();
  }
  if (digitalRead(inst2Pin)==LOW){
    Serial.println("made it");
    controlChange(0, 68, 127);//  (channel, CC number,  CC value)
    MidiUSB.flush();
    //delay();
    //controlChange(0, 64, 0);//  (channel, CC number,  CC value)
    //MidiUSB.flush();
  }
  else {
    Serial.println("off");
    controlChange(0, 68, 0);//  (channel, CC number,  CC value)
    MidiUSB.flush();
  }
  if (digitalRead(inst3Pin)==LOW){
    Serial.println("made it");
    controlChange(0, 69, 127);//  (channel, CC number,  CC value)
    MidiUSB.flush();
    //delay();
    //controlChange(0, 64, 0);//  (channel, CC number,  CC value)
    //MidiUSB.flush();
  }
  else {
    Serial.println("off");
    controlChange(0, 69, 0);//  (channel, CC number,  CC value)
    MidiUSB.flush();
  }
}

void determineNote(){
  distance_cm = mySensor.distance(); //get a distance measurement and store it as distance_cm:
  if (distance_cm <= maxPlayableDistance && distance_cm >= minPlayableDistance){
    assingDistanceToPentatonic = map(distance_cm, minPlayableDistance, maxPlayableDistance, 1, 24); //1-24 is the size of the pentatonic scales
    noteToBePlayed = pentatonic[assingDistanceToPentatonic]; //map() function is used to get an index for the pentatonic scale
    //noteToBePlayed = map(distance_cm, 10, 80, upperNote, lowerNote);
    Serial.println(noteToBePlayed);
    someoneIsPlaying = millis();
  }
  else{
    noteToBePlayed = 0;
    //Serial.println("Not playing");
  }
  oldNote = noteToBePlayed;
}

void turnOn(){
  if (k <= listLength && noteToBePlayed != 0){
    delayNote = millis()+delayTime;
    noteOn(0, noteToBePlayed, 127);
    MidiUSB.flush();
    listOfNotesToTurnOff[k] = noteToBePlayed;          //add note to top of the list
    timesToTurnOffNotes[k] = millis() + noteDuration;
    k++;
  }
}

void turnOff(){
  if (noteToBePlayed != 0){
    if (millis() > timesToTurnOffNotes[0] || k > listLength){ //checks if time is met 
      noteOff(0, listOfNotesToTurnOff[0],0); //turn off index 0, top of the list
      MidiUSB.flush();
      for (j = 0; j < listLength; j++){ //re order index so next note to turn off is at the top of the list
        listOfNotesToTurnOff[j] = listOfNotesToTurnOff[j+1]; //use j to start index from 0, make index 1 --> 0 (top of list)
        timesToTurnOffNotes[j] = timesToTurnOffNotes[j+1];
      }
    k--;
    }
  }
}

void lights(){
  uint32_t greenish = strip.Color(95, 214, 57); //Color displayed
  uint32_t black = strip.Color(0, 0, 0); //Light off
  if (distance_cm <= maxPlayableDistance && distance_cm >= minPlayableDistance){
  int height = map(assingDistanceToPentatonic, 1, 24, LEDNum, 0);
  //if (distance_cm <= 70 && distance_cm >= 10){
  //int height = map(noteToBePlayed, lowerNote, upperNote, LEDNum, 0);
    if (height > oldLEDNum){
      for (int a = 0; a < height; a++){
        strip.setPixelColor(a, greenish);
        strip.show(); 
        delay(10);
      }
    }
    else{
      for (int b=oldLEDNum; b>height; b--){
        strip.setPixelColor(b, black);
        strip.show();                          //  Update strip to match
        delay(10);
      }
    }
  oldLEDNum = height;
  }
  if (flag == false && distance_cm > 70){
    sensorTimeOut = millis();
    flag = true;
  }
  if (flag == true && millis() - sensorTimeOut >= interval) {
    strip.clear();
    strip.show();
    flag = false;
  }
}

void autoPlay() {
  uint32_t greenish = strip.Color(95, 214, 57); //Color displayed
  uint32_t black = strip.Color(0, 0, 0); //Light off

  for (int i = 0; i < 41; i++){
    noteOn(0, twinkle[i], 127);
    MidiUSB.flush();
    int height = map(twinkle[i], 24, 81, LEDNum, 0);
    if (height > oldLEDNum){
      for (int a = 0; a < height; a++){
        strip.setPixelColor(a, greenish);
        strip.show(); 
      }
    }
    else{
      for (int b=oldLEDNum; b>height; b--){
        strip.setPixelColor(b, black);
        strip.show();                          //  Update strip to match
      }
    }
    delay(1000);
    oldLEDNum = height;
    noteOff(0,twinkle[i],0); //turn off index 0, top of the list
    MidiUSB.flush();
    determineNote();
    if (noteToBePlayed != 0){
      break;
    }
  }
}

void loop() {
  MIDIButtons();
  determineNote();
  lights();
  if (millis() > delayNote){turnOn();}
  turnOff();
  //Serial.println(distance_cm);
  if (millis() - someoneIsPlaying > timeSinceSomeoneIsPlaying && noteToBePlayed == 0){
    autoPlay();
    someoneIsPlaying = millis();
  }
}

void controlChange(byte channel, byte control, byte value) {

  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void noteOn(byte channel, byte pitch, byte velocity) {

  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {

  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}