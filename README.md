# Teensy-Arduino-Step-Sequencer

Eight-step MIDI sequencer for a Teensy 4.1, based on [Arduino MIDI Step Sequencer](https://github.com/diyelectromusic/sdemp/tree/main/src/SDEMP/ArduinoMIDIStepSequencer). The original used a fixed MIDI channel and a fixed tempo on an Uno or Nano.

This version adds a startup MIDI-channel knob, a tempo knob, USB MIDI as well as serial MIDI, and an Adafruit 8x16 LED matrix. The matrix shows the MIDI channel at boot, the pitch of the current step while running, and the tempo while that knob is being turned.

## Libraries

Install these from the Arduino Library Manager. Teensyduino already provides `Wire` and `usbMIDI`.

- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by Francois Best
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit LED Backpack Library](https://github.com/adafruit/Adafruit_LED_Backpack)
- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) (installed with the LED Backpack library)

Board: Teensy 4.1. In the Arduino IDE, set USB Type to a type that includes MIDI, such as Serial + MIDI.

The step pots use A0–A7, which includes the default SDA and SCL pins, so the matrix is on Wire2: SCL2 is pin 24 and SDA2 is pin 25, address `0x70`. `matrix.begin(0x70, &Wire2)` selects that bus. The Adafruit libraries do not need to be edited.

## Tempo knob and BPM display

`A14` is read at the top of `loop()`.

- Fully counterclockwise, at or below `STOP_READING` (12), stops the sequence, sends note-off, and the matrix shows `OFF`.
- Turn it back up past `RUN_READING` (30) and the pattern continues from the same step.
- Above that stop zone, `map()` turns the reading into a step delay from 1000 ms down to 15 ms. `constrain()` holds it in that range. 1000 ms is 60 BPM. 15 ms is 4000 BPM. One step is one beat: `bpmmath` is `60000 / delay`.

The matrix shows that BPM number only while the knob is moving. A change of more than 6 counts sets `bpmShowUntil` to 1.5 seconds ahead. Until that time, the display prints the tempo at cursor `(3, 5)`.

From 1000 BPM upward the number stays three digits and row 0 carries the thousands. Each mark is 3 pixels with a 1-pixel gap, at columns 0, 4, 8, and 12. One mark is 1000–1999, two marks 2000–2999, up to four marks. The digits are the remainder, with leading zeros, so 1200 reads `200` with one mark and 4000 reads `000` with four marks.

At a 15 ms step the matrix update and the note messages take a noticeable part of the time, so the tempo you hear can sit under the number on the display. Shorten the synth attack and release as well, or each note is cut off before the envelope opens.

## Note display

Pots `A0` through `A7` are the eight steps. `playingNote` selects the current pot. A reading below `MIN_POT_READING` (4) sends note-off and the matrix shows `--`. Any higher reading is mapped onto the `notes[]` list and sent with `MIDI.sendNoteOn()` and `usbMIDI.sendNoteOn()`.

Except during the short BPM display, the matrix prints that pitch as a name such as `C4` or `F#2`, from `noteNames[]` and the MIDI octave. The name is drawn at cursor `(0, 5)`.

The step position is separate from the name. `drawPixel(x, y, LED_ON)` lights one LED on row 7. `x` walks columns 0, 2, 4, 6, 8, 10, 12, and 14 as the eight steps advance. That dot is drawn in every running state, including while the BPM number is on screen.

For the first 5 seconds after boot, before the sequence starts, the same knob sets the MIDI channel and the matrix shows it as `M` plus the channel number.
