# Tectonic Lingering — Sculpture Version: Build Tutorial

A step-by-step walkthrough of building this project, written so someone
with no prior context could follow along. Grows alongside the stages —
this is not a replacement for [docs/roadmap.md](docs/roadmap.md) or the per-stage
notes files in [docs/stage-1-notes.md](docs/stage-1-notes.md), which capture *why* decisions were made. This document
is about *how* to actually do each step.

---

## Setup: preparing your computer for the ESP32-S3

Before writing or running any code, your computer needs to know how to
talk to the Adafruit ESP32-S3 Feather board. This is a one-time setup.

### 1. Install the Arduino IDE
Download and install the Arduino IDE (version 2.x) from
[arduino.cc/en/software](https://www.arduino.cc/en/software) — pick the
installer for your operating system (Windows/Mac/Linux).

### 2. Add ESP32 board support
The Arduino IDE doesn't know about ESP32-based boards by default — you add
support for them via a "board manager URL":

1. Open Arduino IDE → Preferences (Mac: `Arduino IDE` menu → Settings;
   Windows: `File` → Preferences)
2. Find the field labeled **"Additional Boards Manager URLs"**
3. Paste in:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Click OK
5. Go to **Tools → Board → Boards Manager**
6. Search for **"esp32"** (by Espressif Systems)
7. Click **Install** (this may take a few minutes — it's downloading the
   full toolchain, not just a small file)

### 3. Select the correct board
1. Plug in the ESP32-S3 Feather via USB
2. Go to **Tools → Board → esp32 → Adafruit Feather ESP32-S3**
   (if this exact name isn't listed, look for "ESP32S3 Dev Module" as a
   fallback — functionally similar, just less pre-configured for the
   Feather's specific pins)
3. Go to **Tools → Port** and select the port that appeared when you
   plugged in the board (on Mac, something like `/dev/cu.usbmodemXXXX`;
   on Windows, a `COM` port)

### 4. Confirm it's working
1. Go to **File → Examples → 01.Basics → Blink**
2. Click the **Upload** button (right-arrow icon)
3. Wait for "Done uploading" in the console at the bottom
4. The onboard LED on the board should start blinking

If upload fails with a port/connection error: some ESP32-S3 boards need
you to hold the **BOOT** button while plugging in via USB (or while
clicking Upload) to enter programming mode — check the board's silkscreen
labeling if this happens, since exact button names vary slightly by
manufacturer revision.

**Status: completed.** Board support was already installed and the board
was detected correctly (Adafruit Feather ESP32-S3 No PSRAM) as soon as it
was selected in Tools → Board and Tools → Port. Blink uploaded
successfully and the onboard LED blinked — full toolchain (compile →
upload → run) confirmed working.

---

## Stage 1: touch-sensing test (with OLED readout)

**Goal:** confirm capacitive touch sensing works before writing any LED
code, and see the raw values live instead of only through the Serial
Monitor.

### 1. Wire the touch plate
Connect a single wire from pin **A4** to your metal touch plate. No
second wire to GND is needed — capacitive touch only needs the one
connection (see `docs/stage-1-notes.md` for why).

### 2. Wire the OLED display
0.96" I2C OLED, SSD1306 driver, no STEMMA QT connector — wired directly
to header pins:
- VIN/VCC → 3V
- GND → GND
- SDA → SDA
- SCL → SCL

### 3. Install the OLED libraries
In the Arduino IDE: **Tools → Manage Libraries**, search for and install:
- `Adafruit SSD1306`
- `Adafruit GFX Library`
(this will also pull in `Adafruit BusIO` as a dependency)

### 4. Upload the sketch
Open `stage-1-light-touch/01_touch_test/01_touch_test.ino` and upload it
(same Board/Port as the setup step above).

### 5. Confirm it's working
The OLED should show a number that changes as you touch/release the
plate.

**Status: completed** — OLED displays live touch readings correctly.

---

## Stage 1: LED wave pulse test

**Goal:** get the wave pulse pattern running on one strip before wiring
touch into it, so the motion itself can be tuned in isolation.

### 1. Wire the LED strip
- Strip power (V+/GND) from a separate external 5V supply — do not power
  the strip from the board.
- Board GND and strip power supply GND must be tied together (shared
  ground), even though only the strip draws from the external supply.
- Data line: board pin **A5** → strip data-in.
- A ~1000µF capacitor across the strip's V+/GND at its power input, and a
  ~300–470Ω resistor in series on the data line near the board's pin
  (see `docs/stage-1-notes.md` for why these two matter).

### 2. Install the NeoPixel library
In the Arduino IDE: **Tools → Manage Libraries**, search for and install
`Adafruit NeoPixel`.

### 3. Upload the sketch
Open `stage-1-light-touch/02_led_test/02_led_test.ino` and upload it
(same Board/Port as before). Update `NUM_LEDS` at the top if your strip's
LED count differs from what's currently set.

### 4. Confirm it's working
A bright leader should travel down the strip with a soft trailing tail
behind it, looping continuously.

**Status: completed** — confirmed working on hardware after several
rounds of tuning (see `docs/stage-1-notes.md` for what didn't work along
the way and the final parameter values).
