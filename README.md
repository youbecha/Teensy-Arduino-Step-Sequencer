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

## Higher BPM and thousand marks

The sketch on `main` stops at one step every 150 ms, which is 400 BPM. That limit is the number in the tempo map, not a hardware limit. Step time in milliseconds is `60000 / BPM`. Both `150` entries have to change together. For a 4000 BPM top end, 15 ms is the value:

```cpp
int bpm = map(bpmread, RUN_READING, 1023, 1000, 15);
bpm = constrain(bpm, 15, 1000);
```

The slow end stays 1000 ms, which is 60 BPM. The display math already follows whatever delay you set.

Picopixel draws the BPM on rows 1 through 5 of the rotated 16×8 matrix, so a four-digit number does not fit. Keep three digits and use row 0 for the thousands. Each mark is 3 pixels wide with a 1-pixel gap, at columns 0, 4, 8, and 12. One mark means 1000–1999, two marks 2000–2999, up to four marks. The digits show the remainder, with leading zeros, so 4000 reads `000` with four marks and 1200 reads `200` with one mark. The step dot stays on row 7.

Replace the block that prints `bpmshow` with:

```cpp
int thousands = bpmshow / 1000;
if (thousands > 4) thousands = 4;
int shown = bpmshow % 1000;

matrix.setFont(&Picopixel);
matrix.setTextSize(1);
matrix.setTextWrap(false);
matrix.setTextColor(LED_ON);
matrix.setRotation(1);
matrix.clear();
matrix.setCursor(3, 5);
if (thousands > 0) {
  if (shown < 100) matrix.print('0');
  if (shown < 10) matrix.print('0');
}
matrix.print(shown);
for (int mark = 0; mark < thousands; mark++) {
  matrix.drawFastHLine(mark * 4, 0, 3, LED_ON);
}
matrix.drawPixel(x, y, LED_ON);
matrix.writeDisplay();
```

At 15 ms the matrix update and the note messages take a noticeable part of the step, so the tempo you hear runs under the number on the display. That is a useful check of how fast this hardware actually keeps up. Shorten the synth attack and release as well, or each note is cut off before the envelope opens.

