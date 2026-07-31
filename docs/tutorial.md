# Tectonic Lingering — Sculpture Version: Build Tutorial

A step-by-step walkthrough of building this project, written so someone
with no prior context could follow along. Grows alongside the stages —
this is not a replacement for `docs/roadmap.md` or the per-stage
`notes.md` files, which capture *why* decisions were made. This document
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

**Status: not yet completed — this is the current next step.**
