# Stage 1 — Light + Touch — Notes

## Hardware decisions

**Microcontroller:** Adafruit ESP32-S3 Feather

Why: the ESP32-S3 will run the whole sculpture (touch + LEDs +
audio via the Music Maker FeatherWing), so building Stage 1 directly on it
means the touch/LED code carries forward as-is into later stages, rather
than being rewritten when moving from one board to another.

**IMPORTANT NOTE:** the ESP32-S3's touch sensing reads in the **opposite
direction** compared to the original ESP32 when touched. Online examples 
are often written for the older chip — if touch
detection logic seems inverted, this is why, not a wiring fault.

**LED strip:** WS2812B/NeoPixel-style addressable strip, 42 LEDs
(current test strip — final count may still change, would also change
the power draw calculation below).

## Powering note

Most addressable LED strips expect a **5V** data signal. This *often* still 
works fine over short wire runs, but can cause flickering or glitchy pixels, 
especially over longer runs.

- If pixels behave oddly once wired up: this mismatch is the first thing to
  suspect.
- Fix: a logic level shifter chip (cheap, small) between the ESP32 data pin
  and the strip's data-in line.
- Not fixing preemptively — wiring it directly first is reasonable, this is
  just here so it's not a mystery if it happens.

**Actual power wiring (confirmed):** the ESP32-S3 board itself stays
powered via USB from the computer. The LED strip is powered separately
from an external 5V 3A supply. The two power sources share a common
GND with each other (required for the data signal to have a valid
reference) — only power (V+) comes solely from the 5V supply to the
strip, not from the board.

**Power draw check (42 LEDs):** 42 × 60mA = 2.52A max at full white
brightness on every pixel. The 5V 3A supply gives ~19% headroom above
that — workable but not generous, worth keeping in mind if brightness
ever gets pushed to the limit.

**Data pin:** A5 (GPIO8, touch channel T8) — free of conflicts with the
touch plate (A4), touch indicator LED (GPIO5), and the OLED's I2C
(GPIO3/4/7).

**Protective components added:** ~1000µF capacitor across the strip's
V+/GND at its power input (smooths current draw when many LEDs switch
at once), and a ~300–470Ω resistor in series on the data line near the
GPIO (reduces ringing, protects the first pixel's input).

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

**Wiring:** only a single wire is needed, from A4 to the metal plate —
no GND wire to the plate. Capacitive touch works by the pin measuring
capacitance against the board's own ground reference; touching the plate
adds your body's capacitance to that measurement, no closed loop needed.

**Test sketch:** `stage-1-light-touch/01_touch_test/01_touch_test.ino` —
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

**Touch indicator LED:** external LED on GPIO5 (labeled A8, touch
channel T5) — free, no conflicts. Lights when `touchRead()` exceeds the
100,000 threshold. Confirmed working on hardware.

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
- A single symmetric falloff (brightness = f(|distance from peak|))
  can't produce a "leader" — it grows in equally from both sides. A
  directional effect needs a *signed* distance (ahead vs. behind the
  peak treated differently).
- The position-offset trick (below) breaks down if the offset range
  exceeds the glow's own width — LEDs end up effectively decoupled from
  their physical position and the shape dissolves into noise.

**How the final effect works** (`stage-1-light-touch/02_led_test/`):
- Brightness is gamma-corrected (`pow(brightness, 2.8)`) so fades read
  as smooth rather than stepped, matching how eyes perceive light.
- Each LED has a small, fixed position offset chosen once at startup
  (not re-randomized per frame), so neighboring LEDs don't peak in
  strict index order as the leader passes — this is what gives the
  "catching light unevenly" texture while each LED's own brightness
  still rises/falls perfectly smoothly over time.
- Brightness uses a *signed* distance from the leader's position: a
  short `leadWidth` ahead (sharp front edge) and a longer `tailWidth`
  behind (soft trailing fade) — this asymmetry is what makes it read as
  a leader with a tail rather than a glow expanding from the middle.
- Tuned values: `tailWidth = 4.0`, `leadWidth = 2.0`, `pulseSpeed = 0.3`,
  `offsetRange = 4.0`, `GAMMA = 2.8`, brightness capped at 150/255.
- Color is plain white (kept intentionally — "fire-like" was about the
  organic/warm *quality of motion*, not literally changing hue).

**Touch behavior:** touching the plate should **pause/freeze** the pattern
(hold last state) rather than stop or reset it. Releasing resumes the motion
from where it paused. (Not yet wired into this sketch — still a separate
step, see Open items.)

## Open items

- [x] Confirm the ESP32-S3 Feather development setup is working on the computer
- [x] Write and test a basic touch-sensing sketch (with OLED readout, `stage-1-light-touch/01_touch_test/`) — confirmed working on hardware
- [x] Confirm whether a separate 5V power supply is needed for the strips — yes, 5V 3A external supply, wired and confirmed
- [x] Build and test the wave pulse pattern on one strip (`stage-1-light-touch/02_led_test/`) — confirmed working on hardware
- [ ] Wire touch pause/resume into the LED pattern (currently two separate sketches)
- [ ] Decide whether strips 2 and 3 will use their own data pins or be chained (strip 1 uses A5)
