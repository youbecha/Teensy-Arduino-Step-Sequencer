# Teensy-Arduino-Step-Sequencer
Step Sequencer based on https://github.com/diyelectromusic/sdemp/tree/main/src/SDEMP/ArduinoMIDIStepSequencer

The original has fixed MIDI channel, fixed BPM (beats per minute) and written for Uno or Nano.

I took the original code (which works just fine) and added a knob to select MIDI channel upon boot, and allow adjustable BPM while the sequencer plays.

So that I knew what MIDI channel was used or the actual BPM, I added an Adafruit 8x16 Matrix LED display.

Finally I added code to allow MIDI over USB (a feature of Teensy Arduino).

Fully counterclockwise on the BPM knob stops the sequence, sends note-off, and shows OFF on the matrix. Turn it up past that stop zone and the pattern continues from the same step.

## Libraries

Install these from the Arduino Library Manager. Teensyduino already provides `Wire` and `usbMIDI`.

- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by Francois Best
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit LED Backpack Library](https://github.com/adafruit/Adafruit_LED_Backpack)
- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) (installed with the LED Backpack library)

Board: Teensy 4.1. In the Arduino IDE, set USB Type to a type that includes MIDI, such as Serial + MIDI. The 8x16 matrix is on Wire2 (SCL2 pin 24, SDA2 pin 25) at address 0x70. The sketch passes `&Wire2` into `matrix.begin()`, so the Adafruit libraries do not need to be edited.

