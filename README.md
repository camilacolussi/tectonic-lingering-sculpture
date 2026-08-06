# Tectonic Lingering — Sculpture Version (2026)

Tectonic Lingering is a sound-light sculpture connected to real-time earthquake data. 

This version of the work involves an upgrade in hardware and software from the [installation version](https://www.camilacolussi.com/tectoniclingering) 

## Making focus

One sculpture, built in stages:
1. **Light + touch** — 3 addressable LED strips (30 LEDs each), a wave
   pulse pattern, paused/resumed by touching a metal plate
2. **Sound** — MP3 playback triggered by touch, with reverb/echo behavior
   depending on how quickly you touch back after releasing
3. **Real-time data** — live earthquake magnitude from USGS, overriding
   the normal pattern when a large earthquake is detected

See [docs/tectonic-lingering-sculpture-roadmap.md](docs/tectonic-lingering-sculpture-roadmap.md)
for the full plan, decisions made, and open questions at each stage.

## Hardware

- Adafruit ESP32-S3 Feather (main board — touch, LEDs, and eventually audio)
- Adafruit Music Maker FeatherWing (VS1053 audio codec, MP3/WAV playback via SD card)
- 3x addressable LED strip, 30 LEDs each (WS2812B-style)

## Structure

```
docs/                  Project roadmap and build log
stage-1-light-touch/   Code + notes for the LED/touch stage
stage-2-sound/         Code + notes for the sound stage
stage-3-api/           Code + notes for the earthquake-data stage
media/                 Photos/videos of the physical build
```


