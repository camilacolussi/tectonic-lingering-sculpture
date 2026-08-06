# Tectonic Lingering — Sculpture Version
### Project Roadmap & Build Log

**Reference:** Original room-scale installation > sound-light installation connected to real-time earthquake data, where light "lingers" in expectation and sound tells stories about time and tremors.

**This version:** A single sculptural object exploring the same conceptual core — anticipation, touch, memory, tremor — at object scale rather than room scale.

---

## Architecture (changed 2026-08-06)

Originally planned as one ESP32-S3 board running everything (touch, LEDs,
and eventually sound) in sequential build stages. Restructured into
**two independent boards/processes**, each with its own notes file:

- **[System 1 — API + LEDs](system-1-api-leds-notes.md):** live earthquake
  data from USGS drives the LED wave pulse pattern across the 3 strips.
  Target board: **ESP32 V2** (Adafruit HUZZAH32 – ESP32 Feather V2 —
  confirm exact model once in hand). Not yet owned; arrives 2026-08-07.
  Being built and tested today on the ESP32-S3 Feather as a stand-in.
- **[System 2 — Touch + Sound](system-2-sound-touch-notes.md):** touch
  triggers/controls MP3 playback with reverb/echo behavior. Board:
  Adafruit ESP32-S3 Feather (already in hand, already wired for touch).

The two systems run **independently** — no wireless link between them.

> **Open question, not yet decided:** in the original single-board
> design, touching the plate paused/froze the LED pattern. With touch
> and LEDs now on separate boards running independently, that link is
> broken unless the two boards are given a wireless connection (e.g.
> ESP-NOW — the same idea previously descoped from the old
> multi-sculpture sync plan). Until decided, System 1's LED pattern runs
> autonomously off the API data alone, with no touch input. Revisit this
> once both systems are working independently.

---

## System 1 — API + LEDs

See `docs/system-1-api-leds-notes.md` for full hardware detail and
open items. Summary:

**Goal:** Three addressable LED strips (41 LEDs each) running the wave
pulse pattern continuously. Live magnitude data from the USGS earthquake
feed overrides the normal pattern — magnitude >7.0 detected → all LEDs on
for a duration proportional to magnitude (7.0 → 1 sec, 9.0 → 1 min), then
return to the running pattern.

**Decisions made:** wave pulse shape (leader + trailing tail, tuned on
hardware), LED strip type/count (WS2812B-style, 41 LEDs), power wiring
(separate 5V 3A supply, shared ground).

**Decisions still to make:**
- [ ] Wiring 3 strips from one board — own data pin per strip (recommended default) vs. chained
- [ ] Confirm ESP32 V2 board model + pinout once in hand
- [ ] Whether touch-pauses-LEDs is revisited via a wireless link (see Architecture note above)

### Learning-first approach for the API piece (your request)
Before touching code:
1. **What an API is**, using a few different examples (not just USGS) so the concept generalizes
2. **Data formats** — CSV vs JSON/GeoJSON, what USGS actually offers, why one might suit a microcontroller better than another
3. **How a microcontroller *requests* data** — polling on a timer vs. other approaches
4. Only then: how we extract just the magnitude value and map it to a duration

### The eduroam problem — solvable without solving eduroam
Microcontrollers historically struggle with enterprise WiFi (WPA2-Enterprise, which eduroam uses) — it's not just you. Practical options to discuss:
- [ ] A small always-on device (e.g. Raspberry Pi or even a phone hotspot) that *can* authenticate to eduroam or your home network, acts as a "gateway," fetches the earthquake data, and passes just the relevant number to the ESP32 V2 over a simple local connection
- [ ] Or: skip eduroam entirely and use your own home WiFi / a mobile hotspot for development and even for showing the piece

> **Honest note:** We're routing *around* the eduroam problem, not solving it. Getting a microcontroller to authenticate onto enterprise WiFi (WPA2-Enterprise) directly is a genuinely gnarly rabbit hole that isn't worth your time for this project — a small gateway device sidesteps it entirely.

**Status: In progress.** LED wave pulse pattern confirmed working on
hardware (as a stand-in on the S3). API piece not yet started — that's
the current focus.

---

## System 2 — Touch + Sound

See `docs/system-2-sound-touch-notes.md` for full hardware detail and
open items. Summary:

**Goal:** MP3 track triggered by touch. Stops when released. If released
<5 sec then touched again → resume with reverb/echo added. If released
≥5 sec → track resets to start on next touch.

**Hardware confirmed:** Adafruit ESP32-S3 Feather + Adafruit Music Maker
FeatherWing (VS1053 codec, plays MP3/WAV/etc. over SPI, SD card storage).
Already owned.

### Why sound is the hard part
Real-time audio effects (reverb, echo/delay) are computationally heavier
than blinking LEDs — this is likely why it stalled last time on a bare
ESP32.

**Recommended approach — sidestep live DSP entirely:** the VS1053 chip
handles playback/volume/bass/treble but isn't built for live reverb/echo.
Rather than fight the hardware, pre-render two versions of the track
using Max MSP (which you already know how to do): a "dry" version and a
version with reverb/echo already baked in. Store both on the SD card. The
microcontroller's job becomes simple: decide *which file to play* based
on touch timing, not compute any audio effect live.

**Decisions still to make:**
- [ ] Sound file format & storage: SD card confirmed (FeatherWing has a slot). WAV vs MP3 — MP3 saves space, WAV is simpler if any manipulation is needed beyond swapping pre-rendered files

### Concepts to learn here
- Basics of digital audio effects: what reverb and echo/delay actually *are* technically (not just "how they sound")
- State machines — this is really a state machine (playing → paused-with-effect → stopped), a fundamental concept worth understanding well

**Status: Touch sensing confirmed working on hardware. Sound not yet
started.**

---

## Working method going forward

- Each system is tackled **on its own**, fully, before circuiting back to cross-system questions (like touch-pauses-LEDs).
- Within a system: concept explanation first → component/board decisions → design exploration → code, built incrementally → you test on hardware → report back what happened → iterate.
- Code lives in `system-1-api-leds/` and `system-2-sound-touch/`, each with numbered sketch folders, so we keep clean versions as things evolve.
- This document is our shared map — update the checkboxes and Status lines as we go. Detailed hardware notes and reasoning go in each system's `notes.md` file, not here.

---

## Immediate next step
System 1 (API + LEDs): start the learning-first walkthrough above — what
an API is, with a few examples — before writing any fetch code, since
that's the piece we haven't touched yet.
