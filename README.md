# Teensy-Arduino-Step-Sequencer
Step Sequencer based on https://github.com/diyelectromusic/sdemp/tree/main/src/SDEMP/ArduinoMIDIStepSequencer

The original has fixed MIDI channel, fixed BPM (beats per minute) and written for Uno or Nano.

I took the original code (which works just fine) and added a knob to select MIDI channel upon boot, and allow adjustable BPM while the sequencer plays.

So that I knew what MIDI channel was used or the actual BPM, I added an Adafruit 8x16 Matrix LED display.

Finally I added code to allow MIDI over USB (a feature of Teensy Arduino).

