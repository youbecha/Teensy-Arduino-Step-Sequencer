/*
// Simple DIY Electronic Music Projects
//    diyelectromusic.wordpress.com
//
//  Arduino MIDI Step Sequencer
//  https://diyelectromusic.wordpress.com/2021/01/02/arduino-midi-step-sequencer/
//
      MIT License
      
      Copyright (c) 2020 diyelectromusic (Kevin)
      
      Permission is hereby granted, free of charge, to any person obtaining a copy of
      this software and associated documentation files (the "Software"), to deal in
      the Software without restriction, including without limitation the rights to
      use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
      the Software, and to permit persons to whom the Software is furnished to do so,
      subject to the following conditions:
      
      The above copyright notice and this permission notice shall be included in all
      copies or substantial portions of the Software.
      
      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
      FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
      COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHERIN
      AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
      WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
  Using principles from the following Arduino tutorials:
    Analog Input  - https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInput
    Arduino MIDI Library - https://github.com/FortySevenEffects/arduino_midi_library
*/

/* Modified by Mike, for Teensy 4.1, added an inputs for midi select and for BPM adjust...
 * also to utilize adafruit LED Matrix display
 * the code intially waits for about 5 sec for user to set midi channel, while it displays the midi channel i.e. "M9"
 * then the sequencer starts. The step pots use A0-A7, which includes the default SDA/SCL pins, so the matrix is on Wire2
 * (SCL2 pin 24, SDA2 pin 25). Pass &Wire2 to matrix.begin(); the Adafruit libraries stay stock.
 * A14 sets the MIDI channel for the first 5 seconds, then sets BPM. Fully counterclockwise stops
 * the sequence and sends note-off.
 * Standard Teensy Midi setup.
 */
#include <MIDI.h> //you can either use usb for midi output or wire up a midi circuit using TX1 and RX1 on T4.1
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Fonts/Picopixel.h>
//#include <Fonts/Font4x7Fixed.h>

//added code to run matrix display...if using segment led then edit as necessary
// Matrix is on Wire2. matrix.begin() is given &Wire2 so the Adafruit libraries stay stock.
Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
//back to step sequencer code

#define NUM_POTS 8  // Starting at A0
#define MIN_POT_READING 4 // Value for the lowest note

// This is required to set up the MIDI library.
// The default MIDI setup uses the Arduino built-in serial port
// which is pin 1 for transmitting on the Arduino Uno.
MIDI_CREATE_DEFAULT_INSTANCE();

//int midiChannel = 1;  // Define which MIDI channel to transmit on (1 to 16)...not used with my code
int midiChannel;
int midird;
int numNotes;
int playingNote;
int lastNote = -1;
float bpmmath; // used to get a BPM to display

//next section is setup for the midi channel select code
unsigned long start_millis;  // use this variable to take a snapshot of
                             //    the current number of milliseconds
                             //    since the Teensy last booted

#define HOW_LONG_TO_WAIT_IN_MILLISECONDS  5000  // use this constant to specify how long
                                                //    you want to look/wait for something
                                                //    at startup: this defines 5-seconds
//end of midi channel select code setup stuff

//back to step sequencer code
// Set the MIDI codes for the notes to be played
int notes[] = {
9,10,11,12,  13,14,15,16, 17,18,19,20,  // A- to G#0
21,22,23,24, 25,26,27,28, 29,30,31,32,  // A0 to G#1
33,34,35,36, 37,38,39,40, 41,42,43,44,  // A1 to G#2
45,46,47,48, 49,50,51,52, 53,54,55,56,  // A2 to G#3
57,58,59,60, 61,62,63,64, 65,66,67,68,  // A3 to G#4
69,70,71,72, 73,74,75,76, 77,78,79,80,  // A4 to G#5
81,82,83,84, 85,86,87,88, 89,90,91,92,  // A5 to G#6
93,94,95,96, 97,98,99,100, 101,102,103,104, // A6 to G#7
105,106,107,108, 109,110,111,112, 113,114,115,116, // A7 to G#8
};

void setup() {
  
  Serial.begin(9600);
//  we are not using the default SCL SDA ports, so we need to define Wire2
  Wire2.begin();

//added code to drive matrix LED and the midi channel select code  
  matrix.begin(0x70, &Wire2);  // address 0x70 on SCL2/SDA2, not the default Wire bus

  // next section allows a delay for user to set midi channel using MIDI/BPM pot
  start_millis = millis();   // take a snapshot of the current number of
                              //    milliseconds since the Teensy booted
  while ((millis() - start_millis) < HOW_LONG_TO_WAIT_IN_MILLISECONDS)
   {
      // whatever you put here will be done repeatedly
      //    until your defined time in milliseconds expires
  midird = analogRead(A14);
  //Serial.print("analog 15 is: ");
  //Serial.println(midird);
  midiChannel = map(midird, 0, 1023, 1, 16); //map() converts the analog pot to 16 choices
  //Serial.println(midiChannel);
  delay(50);
//  matrix.setFont(&Font4x7Fixed);
  matrix.setFont(&Picopixel);
  matrix.setTextSize(1);
  matrix.setTextWrap(true);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1); //if hookup wires are on left then 1...if on right then 3
    matrix.clear();
    matrix.setCursor(1,5);
    matrix.print("M ");
    matrix.print(midiChannel);
    matrix.writeDisplay();
    delay(50);
   }
// end of midi channel select and matrix setup code


// back to step sequencer code  https://diyelectromusic.wordpress.com/2021/01/02/arduino-midi-step-sequencer/ 
  // This is a programming trick to work out the number of notes we've listed
  numNotes = sizeof(notes)/sizeof(notes[0]);

  playingNote = 0;
//  MIDI.turnThruOff();
  MIDI.begin(MIDI_CHANNEL_OFF);
}

void loop() {
  const int STOP_READING = 12;  // fully CCW
  const int RUN_READING  = 30;  // turn up past this to run
  static bool sequencerRunning = true;

  // MIDI Controllers should discard incoming MIDI messages...was locking up a synth without it
  while (usbMIDI.read()) {
    // read & ignore incoming messages
  }

  int bpmread = analogRead(A14);
  if (sequencerRunning && bpmread <= STOP_READING) sequencerRunning = false;
  if (!sequencerRunning && bpmread >= RUN_READING) sequencerRunning = true;

  if (!sequencerRunning) {
    if (lastNote != -1) {
      MIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
      usbMIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
      lastNote = -1;
    }

    matrix.setFont(&Picopixel);
    matrix.setTextSize(1);
    matrix.setTextWrap(false);
    matrix.setTextColor(LED_ON);
    matrix.setRotation(1);
    matrix.clear();
    matrix.setCursor(0, 5);
    matrix.print("OFF");
    matrix.writeDisplay();

    delay(30);
    return;
  }

  // Loop through each note in the sequence in turn, playing
  // the note defined by the potentiomer for that point in the sequence.

  if (playingNote >= NUM_POTS) {
    playingNote = 0;
  }
  //tracks where in the sequence it is for use on display later in the code...
  int x = (((playingNote + 1) * 2) - 2);
  int y = 7;
  
  // Take the reading for this pot
  int potReading = analogRead (A0+playingNote);
  int midiNow = -1;

  // if the reading is zero (or almost zero), turn off any playing note
  if (potReading < MIN_POT_READING) {
    if (lastNote != -1) {
      MIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
      usbMIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
      lastNote = -1;
      
    }
  } else {
    // This is a special function that will take one range of values,
    // in this case the analogue input reading between 0 and 1023, and
    // produce an equivalent point in another range of numbers, in this
    // case choose one of the notes in our list.
    int playing = map(potReading, MIN_POT_READING, 1023, 0, numNotes-1);
    midiNow = notes[playing];

    // If we are already playing a note then we need to turn it off first
    if (lastNote != -1) {
        MIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
        usbMIDI.sendNoteOff(notes[lastNote], 0, midiChannel);
    }
    MIDI.sendNoteOn(notes[playing], 127, midiChannel);
    usbMIDI.sendNoteOn(notes[playing], 127, midiChannel);
    lastNote = playing;
    
 // Serial.print("playing= ");
 // Serial.println(x);
  }
  
  // Move on to the next note after a short delay.
  // 250 = 1/4 of a second so this is same as 240 beats per minute.
  // so this next section reads A14 and creates a ms delay as well as a BPM display
  // had to use float and remember to add decimal points so the math would work

  // A14 sets the step delay. Fully CCW is handled above and never reaches here.
  // 1000ms is 60 BPM. 15ms is 4000 BPM.
  int bpm = map(bpmread, RUN_READING, 1023, 1000, 15);
  bpm = constrain(bpm, 15, 1000);
  float bpmmath = 1.0 / (bpm / 1000.0) * 60.0; //probably a better way to do this, but it is what I came up with
  int bpmshow = bpmmath;

  // Show BPM only while the A14 knob is being turned. Otherwise show the pitch.
  static int lastBpmRead = -1;
  static uint32_t bpmShowUntil = 0;
  if ((lastBpmRead >= 0) && (abs(bpmread - lastBpmRead) > 6)) {
    bpmShowUntil = millis() + 1500;
  }
  lastBpmRead = bpmread;

  matrix.setFont(&Picopixel);
  matrix.setTextSize(1);
  matrix.setTextWrap(false);
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix.clear();
  if ((int32_t)(bpmShowUntil - millis()) > 0) {
    int thousands = bpmshow / 1000;
    if (thousands > 4) thousands = 4;
    int shown = (thousands > 0) ? (bpmshow % 1000) : bpmshow;
    matrix.setCursor(3, 5);
    if (thousands > 0) {
      if (shown < 100) matrix.print('0');
      if (shown < 10) matrix.print('0');
    }
    matrix.print(shown);
    for (int mark = 0; mark < thousands; mark++) {
      matrix.drawFastHLine(mark * 4, 0, 3, LED_ON);
    }
  } else if (midiNow < 0) {
    matrix.setCursor(3, 5);
    matrix.print("--");
  } else {
    static const char *noteNames[12] = {
      "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    char noteText[6];
    snprintf(noteText, sizeof(noteText), "%s%d", noteNames[midiNow % 12], (midiNow / 12) - 1);
    matrix.setCursor(0, 5);
    matrix.print(noteText);
  }
  matrix.drawPixel(x, y, LED_ON);  //uses variables from above to plot a dot at the current step position
    matrix.writeDisplay();
  // the above modified code displays the BPM and a dot below it for each note played...
    
  delay(bpm);
  playingNote++;
}
