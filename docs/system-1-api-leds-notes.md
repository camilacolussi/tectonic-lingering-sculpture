# System 1 — API + LEDs — Notes

## Architecture decision (2026-08-06)

This project restructured from "one sculpture, one board, everything on
it" into **two independent boards/processes**:

- **System 1 (this file):** live earthquake API data → LED wave pulse
  pattern on the 3 strips. Target board: **ESP32 V2** (assumed to be the
  Adafruit HUZZAH32 – ESP32 Feather V2, plain ESP32, not S3 — confirm
  exact model once the board is in hand).
- **System 2:** touch → sound. See `docs/system-2-sound-touch-notes.md`.
  Target board: ESP32-S3 Feather.

**Why split into two boards:** originally one ESP32-S3 was going to run
touch + LEDs + audio together. Decided to separate into two independent
systems instead — reasoning/tradeoffs not yet fully logged here, revisit
if it matters later.

**Open question — touch no longer pauses the LED pattern:** in the
original single-board design, touching the plate paused/froze the LED
wave pulse. With touch living on System 2's board and the LEDs on
System 1's board, and the two systems running independently (no
wireless link between them), that interaction can't happen anymore as
originally designed. Options if this is wanted later: a wireless link
between the two boards (e.g. ESP-NOW — this is the same idea that was
descoped from the old "Stage 4" multi-sculpture sync plan), or drop the
touch-pauses-LEDs behavior entirely and let System 1 run autonomously
off the API data alone. **Not yet decided — flag before assuming either
way.**

**Hardware arrived (2026-08-07):** the ESP32 V2 board is in hand — it's
the Adafruit Feather ESP32 V2 (chip identifies as ESP32-PICO-V3-02).
Until now, System 1 code was written and tested on the ESP32-S3 Feather
as a stand-in (it also has WiFi, so the API-fetching logic ports over
directly; only pin numbers may need to change now that code moves to the
actual V2 board).

**Uploading to the Feather ESP32 V2 — no manual bootloader button:**
unlike some other ESP32 boards, this board has no BOOT/GPIO0 button to
manually force download mode. Its two buttons are `Reset` (reboots the
board) and `SW38` (a general-purpose user input button on GPIO38,
exposed in Arduino as `BUTTON`, pulled up on-board) — SW38 is *not* tied
to bootloader entry. Uploading relies entirely on the automatic DTR/RTS
reset circuit built around the onboard CP2104 USB-serial chip.

- First upload attempt (Blink) failed partway through: esptool connected
  and correctly identified the chip, but then failed with "Unable to
  verify flash chip connection" during the stub-flasher step — a sign
  the auto-reset timing into bootloader mode wasn't reliable at the
  default upload speed.
- **Fix that worked:** Tools → Upload Speed → 115200 (down from the
  default 921600). Board has no power LED or power switch, so visual
  power-good confirmation isn't available on this board either — keep
  that in mind if a board ever appears totally unresponsive.
- The CP2104 driver itself was *not* the issue here — esptool reaching
  the chip-ID step at all confirms the driver was already working; no
  separate driver install was needed on this Mac.

## LED strip

**Type:** WS2812B/NeoPixel-style addressable strip, 41 LEDs (confirmed
count) — 3 strips total, one per side of the sculpture.

## Powering note

Most addressable LED strips expect a **5V** data signal. This *often*
still works fine over short wire runs, but can cause flickering or
glitchy pixels, especially over longer runs.

- If pixels behave oddly once wired up: this mismatch is the first thing
  to suspect.
- Fix: a logic level shifter chip (cheap, small) between the ESP32 data
  pin and the strip's data-in line.
- Not fixing preemptively — wiring it directly first is reasonable, this
  is just here so it's not a mystery if it happens.

**Actual power wiring (confirmed, on the ESP32-S3 stand-in):** the board
itself stays powered via USB from the computer. The LED strip is powered
separately from an external 5V 3A supply. The two power sources share a
common GND with each other (required for the data signal to have a valid
reference) — only power (V+) comes solely from the 5V supply to the
strip, not from the board. Re-confirm this wiring once moved to the
actual V2 board.

**Power draw check (41 LEDs):** 41 × 60mA = 2.46A max at full white
brightness on every pixel. The 5V 3A supply gives ~22% headroom above
that — workable but not generous, worth keeping in mind if brightness
ever gets pushed to the limit. This scales per strip — with 3 strips
eventually running, check whether one shared 5V supply has enough
headroom for all three, or whether each strip needs its own supply.

**Data pin (on the ESP32-S3 stand-in):** A5 (GPIO8). Re-confirm against
the V2 board's actual pinout once it's in hand — pin numbering is not
guaranteed to carry over between an S3 and a plain ESP32.

**Protective components added:** ~1000µF capacitor across the strip's
V+/GND at its power input (smooths current draw when many LEDs switch at
once), and a ~300–470Ω resistor in series on the data line near the GPIO
(reduces ringing, protects the first pixel's input).

## Lighting pattern decision

**Wave pulse, final shape (tuned on hardware):** a bright leader travels
the strip with a soft trailing tail behind it — asymmetric, not a
symmetric glow growing from the middle outward. Reached through
iterative on-hardware testing; what didn't work along the way is worth
keeping for next time:
- Re-randomizing each LED's brightness *every frame* read as flickery/
  noisy, not organic — texture needed to come from a fixed, one-time
  randomization instead (see position offset below), not continuous
  per-frame noise.
- A single symmetric falloff (brightness = f(|distance from peak|)) can't
  produce a "leader" — it grows in equally from both sides. A directional
  effect needs a *signed* distance (ahead vs. behind the peak treated
  differently).
- The position-offset trick (below) breaks down if the offset range
  exceeds the glow's own width — LEDs end up effectively decoupled from
  their physical position and the shape dissolves into noise.

**How the final effect works** (`system-1-api-leds/01_led_test/`):
- Brightness is gamma-corrected (`pow(brightness, 2.8)`) so fades read as
  smooth rather than stepped, matching how eyes perceive light.
- Each LED has a small, fixed position offset chosen once at startup (not
  re-randomized per frame), so neighboring LEDs don't peak in strict
  index order as the leader passes — this is what gives the "catching
  light unevenly" texture while each LED's own brightness still
  rises/falls perfectly smoothly over time.
- Brightness uses a *signed* distance from the leader's position: a short
  `leadWidth` ahead (sharp front edge) and a longer `tailWidth` behind
  (soft trailing fade) — this asymmetry is what makes it read as a leader
  with a tail rather than a glow expanding from the middle.
- Tuned values: `tailWidth = 4.0`, `leadWidth = 2.0`, `pulseSpeed = 0.3`,
  `offsetRange = 4.0`, `GAMMA = 2.8`, brightness capped at 150/255.
- Color is plain white (kept intentionally — "fire-like" was about the
  organic/warm *quality of motion*, not literally changing hue).

**Breathing effect experiments:** tried three approaches to layering a
slow "breathing" dim/brighten on top of the wave pulse (global brightness
envelope, background-glow-only, additive layer) — none kept, experiment
file was deleted. If revisited, the three approaches and why each
combines the two brightness values differently (multiply / max / add)
are worth re-deriving rather than looking for old code, since the file no
longer exists in the repo.

## API — earthquake data (USGS)

**Confirmed working (2026-08-07)**, in `system-1-api-leds/03_API_test/` —
connects to eduroam, fetches from USGS, prints the latest matching
earthquake's magnitude to Serial once a minute.

- **Network:** the board connects over **eduroam (WPA2-Enterprise)**, not a
  plain home WiFi network — this only became relevant once testing moved
  off the home network the S3 stand-in was originally tested on.
  Connection is handled by the third-party `WiFiEnterprise` library
  (`~/Documents/Arduino/libraries/ESP32WiFiEnterprise`), tested standalone
  first in `system-1-api-leds/02_ESP32_eduroam/` before folding into
  `03_API_test`. That library internally includes the now-deprecated
  `esp_wpa2.h` (current ESP32 core prefers `esp_eap_client.h`) — compiles
  fine today with just a deprecation warning, but flagging in case a future
  core update breaks it.
- **Credential handling — deliberately manual, no file ever holds them:**
  `username`/`password` in both `02_ESP32_eduroam.ino` and
  `03_API_test.ino` are compile-time `const char*` constants, left blank
  (`""`) at rest. Real values are hand-typed in immediately before each
  upload, then blanked back out before saving — never committed, and never
  read by an AI assistant working in this repo. Two approaches were tried
  and rejected first: (1) a separate `arduino_secrets.h` file excluded via
  `.gitignore` — safe from GitHub, but still a plaintext file sitting on
  disk that could be opened; (2) typing credentials at runtime over Serial
  each boot — keeps them out of any file entirely, but requires a live USB
  connection, which conflicts with the board eventually running standalone
  off its own power with no computer attached. Manual pre-upload editing
  was the only option satisfying both "never on disk" and "works
  untethered." **Real risk to stay aware of:** since these two `.ino`
  files aren't gitignored, nothing stops a `git commit` from picking up
  real credentials if they aren't blanked out before saving — this relies
  entirely on remembering the edit → upload → blank → save sequence every
  time, with no automated safety net.
- **Efficiency choices in the USGS request** (`03_API_test.ino`):
  `format=text` (pipe-delimited plain text, not GeoJSON — far smaller
  payload, no JSON library needed), `limit=1&orderby=time` (server returns
  just the single most recent match), `minmagnitude=` filter applied
  server-side. Currently set to **4.5** (raised from an initial 2.5 while
  testing — 2.5+ events are frequent enough to fire almost every fetch).
  Only the `Magnitude` field (pipe-index 10) is parsed out; location was
  dropped since it wasn't needed.
- Upload note specific to this board: Tools → Board → **Adafruit Feather
  ESP32 V2**, Tools → Upload Speed → **115200** (see the Blink upload fix
  logged above — same fix applies here).

## Three strips (`07_quake_LEDtail_mag_x3_sync`)

**Written 2026-09-23, compiles for the Feather ESP32 V2 — not yet tested on hardware.**
Builds on `06_quake_LEDtail_mag`: same wave tail, quake detection and
magnitude-scaled flash, but across 3 strips instead of 1.

- **Own data pin per strip, not chained or split in hardware.** Chaining/splitting
  one data line would guarantee identical strips with almost no code change,
  but locks all 3 into the same length and look forever. Separate pins keep
  each strip independently controllable. Timing cost is negligible: each
  `show()` is ~1.2 ms for 40 LEDs, so all 3 update within ~4 ms of each other,
  against a 40 ms frame.
- **Pins: A5, A0, A1.** On the Feather ESP32 V2, A2/A3/A4 are input-only
  (GPIO 34/39/36) and can't drive a strip. A0/A1 are ADC2 pins (ADC2 analog
  reads don't work while WiFi is on), but that only affects `analogRead`, not
  using them as digital outputs.
- **`randomSeed` moved from A0 to A2**, since A0 is now a strip output — reading
  it would give the same seed every boot, so the shimmer would repeat.
- **Shared wave, separate shimmer.** One `pulsePos` for all strips, so the pulse
  moves in step; a separate `posOffset` row per strip, so each has its own
  texture.
- **LED count: 40 per strip** (was 41 on the single-strip sketches).
- **Power:** 3 x 40 LEDs at full white is roughly 4+ A at brightness 150, and the
  flash can now hold for up to 3 minutes. An external supply rated for this is
  needed. Which supply goes with which stage:
  - **5V 3A** — used for sketches 01–06 (one strip, 41 LEDs). Enough there:
    one strip at full white is ~1.4 A at brightness 150.
  - **5V 30A** — used from 07 onwards (3 strips, 40 LEDs each). The 3A
    supply is no longer enough (~4+ A worst case during the flash); 30A
    leaves a very large margin. Because a 30A supply can push a lot of
    current into a short circuit, a fuse on the 5V line and wire rated for
    the current are worth adding.

## Open items

- [x] Confirm the ESP32-S3 Feather development setup is working on the computer
- [x] Confirm whether a separate 5V power supply is needed for the strip — yes, 5V 3A external supply, wired and confirmed (on the S3 stand-in) — replaced by a 5V 30A supply from 07 onwards (3 strips)
- [x] Build and test the wave pulse pattern on one strip (`system-1-api-leds/01_led_test/`) — confirmed working on hardware
- [x] Confirm final LED count — 40 per strip, 3 strips (was 41 on the single-strip sketches 01–06)
- [x] Decide whether strips 2 and 3 will use their own data pins or be chained — own pins: A5, A0, A1 on the V2 (see Three strips section)
- [ ] Test `07_quake_LEDtail_mag_x3_sync` on hardware with all 3 strips (40 LEDs each)
- [x] Confirm exact ESP32 V2 board model — Adafruit Feather ESP32 V2 (chip: ESP32-PICO-V3-02), in hand and uploading successfully as of 2026-08-07
- [ ] Confirm the V2 board's pinout once wiring the LED strip to it (data pin was A5/GPIO8 on the S3 stand-in — likely needs to change)
- [ ] Re-verify power wiring, data pin, and protective components once moved from the S3 stand-in to the actual V2 board
- [ ] Decide whether touch-pauses-LEDs is dropped, or revisited later via a wireless link between the two boards
- [x] Build and test the USGS earthquake API fetch — confirmed working (`system-1-api-leds/03_API_test/`), prints magnitude to Serial once a minute over eduroam
- [x] Wire the fetched magnitude into the LED wave pulse pattern — combined in `05_quake_LEDtail` (new quake → white flash), then `06_quake_LEDtail_mag` made the flash duration scale with magnitude
- [ ] Add the power supply enclosure (box being built for the 5V 30A supply) to the repo once finished — files/photos/notes, so all physical elements live here too
