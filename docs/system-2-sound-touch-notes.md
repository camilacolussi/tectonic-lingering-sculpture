# System 2 — Touch + Sound — Notes

## Architecture

Part of the two-system split (see `docs/system-1-api-leds-notes.md` for
the full reasoning and the open question about touch no longer pausing
the LED pattern). This system: touch sensing → triggers/controls sound
playback. Runs independently of System 1 (API + LEDs).

**Board:** Adafruit ESP32-S3 Feather.

Why: this board was already being used for the original single-board
design, has the Music Maker FeatherWing already owned for audio, and its
touch-capable pins are already confirmed working (see below) — so it
carries forward directly into this system rather than needing rework.

**IMPORTANT NOTE:** the ESP32-S3's touch sensing reads in the **opposite
direction** compared to the original ESP32 when touched. Online examples
are often written for the older chip — if touch detection logic seems
inverted, this is why, not a wiring fault.

## Touch sensing pin

**Pin:** A4 (GPIO14, touch channel T14), confirmed against the actual
installed board variant file
(`adafruit_feather_esp32s3_nopsram/pins_arduino.h`), not guessed.

Why A4: it's one of the touch-capable channels (T3, T4, T5, T6, T8, T9,
T10, T11, T12, T13, T14 are the only touch-capable GPIOs on this board)
that's also silkscreened directly on the header, and it's free of
conflicts — T13 shares GPIO13 with `LED_BUILTIN`, and T3/T4 share
GPIO3/GPIO4 with I2C `SDA`/`SCL` (kept free for a possible future I2C
sensor via STEMMA QT).

**Wiring:** only a single wire is needed, from A4 to the metal plate — no
GND wire to the plate. Capacitive touch works by the pin measuring
capacitance against the board's own ground reference; touching the plate
adds your body's capacitance to that measurement, no closed loop needed.

**Test sketch:** `system-2-sound-touch/01_touch_test/01_touch_test.ino` —
prints raw `touchRead()` values continuously rather than assuming a
touch/no-touch threshold, so the actual touched vs. untouched range can
be observed on this specific plate/wiring before picking one.

**Measured values (this plate/wiring):**
- Untouched: ~32,030–32,057 (tight, stable noise band)
- Touched: ~228,000–245,000 (drifts upward the longer it's held — likely
  skin contact area/moisture increasing over time)

**Threshold decision: 100,000.** Sits with wide margin above the
untouched noise ceiling and well below the lowest touched reading seen —
small variation in either range won't cause false triggers.

**Touch indicator LED:** external LED on GPIO5 (labeled A8, touch channel
T5) — free, no conflicts. Lights when `touchRead()` exceeds the 100,000
threshold. Confirmed working on hardware.

**IMPORTANT NOTE — board selection matters:** `I2C_POWER` (used below) is
only defined for the "Adafruit Feather ESP32-S3" and "Adafruit Feather
ESP32-S3 No PSRAM" board variants in Tools → Board. The TFT variants
redefine it as `TFT_I2C_POWER` on a different pin, and the generic
"ESP32S3 Dev Module" fallback doesn't define it at all — if you get an
`'I2C_POWER' was not declared in this scope` compile error, check the
board selection first, it's not a code bug.

## OLED for debugging (touch test)

**Display:** 0.96" I2C OLED, SSD1306 driver, 128x64. Used to visualize
raw `touchRead()` values directly on the hardware instead of relying on
the Serial Monitor.

Wiring: SDA → GPIO3, SCL → GPIO4 (the board's default I2C pins) — this is
also why those two touch channels (T3/T4) were avoided for the touch
plate itself, see "Touch sensing pin" above.

**IMPORTANT NOTE:** this board gates power to its I2C bus through a
dedicated pin (`I2C_POWER`, GPIO7). It must be driven HIGH in code before
`Wire.begin()` or the display stays completely unresponsive even when
wired correctly — this isn't obvious from the display's own wiring and
easily reads as a wiring fault if you don't know to look for it.

Libraries used: `Adafruit SSD1306` + `Adafruit GFX Library` (installed
via Library Manager / `arduino-cli lib install`).

## Sound

Not yet started. Per the roadmap: Adafruit Music Maker FeatherWing
(VS1053 codec, MP3/WAV via SD card), already owned. Planned approach:
pre-render dry + reverb/echo versions of the track (in Max MSP) rather
than computing effects live on-device — see `docs/roadmap.md` for the
full reasoning.

## Open items

- [x] Write and test a basic touch-sensing sketch (with OLED readout, `system-2-sound-touch/01_touch_test/`) — confirmed working on hardware
- [ ] Wire up the Music Maker FeatherWing and confirm SD card playback
- [ ] Pre-render dry + reverb/echo track versions in Max MSP
- [ ] Build the touch → playback state machine (playing → paused-with-effect → stopped, per the timing rules in `docs/roadmap.md`)
