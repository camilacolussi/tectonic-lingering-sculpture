# Tectonic Lingering — Sculpture Version
### Project Roadmap & Build Log

**Reference:** Original room-scale installation > sound-light installation connected to real-time earthquake data, where light "lingers" in expectation and sound tells stories about time and tremors.

**This version:** A single sculptural object exploring the same conceptual core — anticipation, touch, memory, tremor — at object scale rather than room scale. One sculpture, richer internally (3 LED strips, touch, sound, live earthquake data) rather than 3 simpler synced sculptures.

---

## Stage 1 — Light + Touch

**Goal:** Three addressable LED strips (42 LEDs each currently — count may
still change, see below — one sculpture, more may be added later) running
a "wave pulse" pattern, paused/frozen while touching a metal plate
(capacitive touch), resuming from where they paused on release.

### Decisions made
- [x] **Microcontroller**: Adafruit ESP32-S3 Feather
- [x] **Lighting pattern**: Wave pulse, refined on hardware into an asymmetric leader-with-trailing-tail shape (not a symmetric glow) — see `docs/stage-1-notes.md` for the tuned parameters and what didn't work along the way
- [x] **Touch behavior**: pause/freeze the pattern on touch, resume from same position on release
- [x] **LED strip type**: WS2812B/NeoPixel-style (individually addressable), confirmed
- [x] **Power**: separate 5V 3A supply for the LED strip(s), confirmed and wired — board itself stays powered via USB from the computer, grounds shared between the two supplies

### Decisions still to make
- [ ] **Final LED count per strip**: 42 confirmed for the current test strip, but may change — changing it means updating both the code (`NUM_LEDS`) and the power draw calculation (LEDs × 60mA)
- [ ] **Wiring 3 strips from one board**: whether each strip gets its own data pin (simplest, most reliable) or they're chained — three separate data pins is the recommended default unless there's a specific reason to chain

### Concepts to learn here
- Capacitive touch sensing — how it senses "touch" without a physical switch
- Basic microcontroller I/O concepts (digital pins vs. touch-capable pins)

### Status: **In progress**

---

## Stage 2 — Sound

**Goal:** MP3 track triggered by touch. Stops when released. If released <5 sec then touched again → resume with reverb/echo added. If released ≥5 sec → track resets to start on next touch.

### Why this is the hard stage
Real-time audio effects (options to be explored) are computationally heavier than blinking LEDs. This is likely why it stalled last time on a bare ESP32.

**Hardware confirmed:** Adafruit ESP32-S3 Feather + Adafruit Music Maker FeatherWing (VS1053 codec, plays MP3/WAV/etc. over SPI, SD card storage). 

**Recommended approach — sidestep live DSP entirely:** the VS1053 chip handles playback/volume/bass/treble but isn't built for live reverb/echo. Rather than fight the hardware, pre-render two versions of the track using Max MSP (which you already know how to do): a "dry" version and a version with reverb/echo already baked in. Store both on the SD card. The microcontroller's job becomes simple: decide *which file to play* based on touch timing, not compute any audio effect live.

### Decisions to make together
- [x] **Board for audio**: Adafruit ESP32-S3 Feather + Music Maker FeatherWing (confirmed, already owned) — same board that runs Stage 1's LEDs/touch, since this is now a single sculpture rather than multiple boards talking to each other
- [ ] **Sound file format & storage**: SD card confirmed (FeatherWing has a slot). WAV vs MP3 — MP3 saves space, WAV is simpler if any manipulation is needed beyond swapping pre-rendered files

### Concepts to learn here
- Basics of digital audio effects: what reverb and echo/delay actually *are* technically (not just "how they sound")
- State machines — this stage is really a state machine (playing → paused-with-effect → stopped) which is a fundamental programming concept worth understanding well, since Stage 3 and 4 will reuse it

### Status: **Not started**

---

## Stage 3 — Real-time API Data (Earthquakes)

**Goal:** Pull magnitude data from USGS feed. Normal state = LEDs "running." Magnitude >7.0 detected → all LEDs on for a duration proportional to magnitude (7.0 → 1 sec, 9.0 → 1 min), then return to running pattern.

### Learning-first approach (your request)
Before touching code:
1. **What an API is**, using a few different examples (not just USGS) so the concept generalizes
2. **Data formats** — CSV vs JSON/GeoJSON, what USGS actually offers, why one might suit a microcontroller better than another
3. **How a microcontroller *requests* data** — polling on a timer vs. other approaches
4. Only then: how we extract just the magnitude value and map it to a duration

### The eduroam problem — solvable without solving eduroam
Microcontrollers historically struggle with enterprise WiFi (WPA2-Enterprise, which eduroam uses) — it's not just you. Practical options to discuss:
- [ ] A small always-on device (e.g. Raspberry Pi or even a phone hotspot) that *can* authenticate to eduroam or your home network, acts as a "gateway," fetches the earthquake data, and passes just the relevant number to your sculpture's microcontroller over a simple local connection
- [ ] Or: skip eduroam entirely and use your own home WiFi / a mobile hotspot for development and even for showing the piece

> **Honest note:** We're routing *around* the eduroam problem, not solving it. Getting a microcontroller to authenticate onto enterprise WiFi (WPA2-Enterprise) directly is a genuinely gnarly rabbit hole that isn't worth your time for this project — a small gateway device sidesteps it entirely.

### Status: **Not started**

---

## Working method going forward

- We tackle **one stage at a time**, fully, before moving to the next.
- At each stage: concept explanation first → component/board decisions → design exploration (e.g. lighting patterns) → code, built incrementally → you test on hardware → report back what happened → iterate.
- Code will live in dedicated code artifacts per stage so we keep clean versions as things evolve.
- This document is our shared map — update the checkboxes and Status lines as we go, and add a **Notes** line under any stage once we hit real-world surprises (wiring issues, part substitutions, things that didn't work as expected).

---

## Immediate next step
Touch sensing and the LED wave pulse pattern are each confirmed working
on hardware, but as two separate sketches. Next: combine them so touch
pauses/freezes the pattern and release resumes it from the same position.
