# Stage 1 — Light + Touch — Notes

## Hardware decisions

**Microcontroller:** Adafruit ESP32-S3 Feather

Why: the ESP32-S3 will run the whole sculpture eventually (touch + LEDs +
audio via the Music Maker FeatherWing), so building Stage 1 directly on it
means the touch/LED code carries forward as-is into later stages, rather
than being rewritten when moving from one board to another.

**Known gotcha:** the ESP32-S3's touch sensing reads in the **opposite
direction** compared to the original ESP32 (used on the HUZZAH32) when
touched. Online examples are often written for the older chip — if touch
detection logic seems inverted, this is why, not a wiring fault.

**LED strip:** 30-LED addressable strip (already in hand). Type/model TBD —
confirm WS2812B or similar when wiring.

## Known technical gotcha (documented in advance)

The HUZZAH32 runs logic at **3.3V**. Most addressable LED strips expect a
**5V** data signal. This *often* still works fine over short wire runs, but
can cause flickering or glitchy pixels, especially over longer runs.

- If pixels behave oddly once wired up: this mismatch is the first thing to
  suspect.
- Fix: a logic level shifter chip (cheap, small) between the ESP32 data pin
  and the strip's data-in line.
- Not fixing preemptively — wiring it directly first is reasonable, this is
  just here so it's not a mystery if it happens.

## Lighting pattern decision

Explored three options (comet/chase, wave pulse, multi-point) as live previews.

**Chosen: Wave pulse** — a soft glow travels the length of the strip,
brightening then dimming as it passes each LED, rather than a single hard
point with a trail. Deliberately different from the comet-style pattern used
in the original build — a conscious choice to explore something new, and it
fits the tectonic/breathing theme of the piece well.

**Touch behavior:** touching the plate should **pause/freeze** the pattern
(hold last state) rather than stop or reset it. Releasing resumes the motion
from where it paused.

## Open items
- [ ] Confirm exact LED strip model/voltage
- [ ] Confirm which HUZZAH32 pin(s) support capacitive touch (check pinout)
- [ ] Wire touch plate + LED strip
- [ ] Write first Arduino sketch: wave pulse pattern, touch pin pauses motion
