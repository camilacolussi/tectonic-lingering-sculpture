# Tectonic Lingering — Sculpture Version (2026)

Tectonic Lingering is a sound-light sculpture connected to real-time earthquake data. 

This version of the work involves an upgrade in hardware and software from the [installation version](https://www.camilacolussi.com/tectoniclingering) 

## Making focus

One sculpture, built as **two independent electronic systems**
(restructured 2026-08-06 from an earlier single-board plan):

1. **System 1 — API + LEDs** — 3 addressable LED strips (41 LEDs each), a
   wave pulse pattern, overridden by live earthquake magnitude data from
   USGS. Board: ESP32 V2.
2. **System 2 — Touch + Sound** — MP3 playback triggered by touch, with
   reverb/echo behavior depending on how quickly you touch back after
   releasing. Board: Adafruit ESP32-S3 Feather.

The two systems run independently — see
[docs/roadmap.md](docs/roadmap.md) for the full plan, decisions made, and
the open question around whether/how touch and the LED pattern interact
now that they're on separate boards.

## Hardware

- ESP32 V2 (Adafruit HUZZAH32 – ESP32 Feather V2, assumed) — System 1, API + LEDs
- Adafruit ESP32-S3 Feather — System 2, touch + sound
- Adafruit Music Maker FeatherWing (VS1053 audio codec, MP3/WAV playback via SD card) — System 2
- 3x addressable LED strip, 41 LEDs each (WS2812B-style) — System 1

## Structure

```
docs/                     Project roadmap, per-system notes, and tutorial
system-1-api-leds/        Code for the API + LEDs system
system-2-sound-touch/     Code for the touch + sound system
media/                    Photos/videos of the physical build
```


