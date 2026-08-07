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

## Open items

- [x] Confirm the ESP32-S3 Feather development setup is working on the computer
- [x] Confirm whether a separate 5V power supply is needed for the strip — yes, 5V 3A external supply, wired and confirmed (on the S3 stand-in)
- [x] Build and test the wave pulse pattern on one strip (`system-1-api-leds/01_led_test/`) — confirmed working on hardware
- [x] Confirm final LED count — 41
- [ ] Decide whether strips 2 and 3 will use their own data pins or be chained (strip 1 uses A5 on the S3 stand-in)
- [x] Confirm exact ESP32 V2 board model — Adafruit Feather ESP32 V2 (chip: ESP32-PICO-V3-02), in hand and uploading successfully as of 2026-08-07
- [ ] Confirm the V2 board's pinout once wiring the LED strip to it (data pin was A5/GPIO8 on the S3 stand-in — likely needs to change)
- [ ] Re-verify power wiring, data pin, and protective components once moved from the S3 stand-in to the actual V2 board
- [ ] Decide whether touch-pauses-LEDs is dropped, or revisited later via a wireless link between the two boards
- [x] Build and test the USGS earthquake API fetch — confirmed working (`system-1-api-leds/03_API_test/`), prints magnitude to Serial once a minute over eduroam
- [ ] Wire the fetched magnitude into the LED wave pulse pattern (currently the two sketches are separate — `01_led_test` for LEDs, `03_API_test` for the API — not yet combined into one program)
