// LED motion test v2: same leader+tail comet as 02_led_test, plus a
// breathing effect running in parallel. Variant A (current): a slow sine
// wave multiplies the WHOLE strip's brightness, so the comet keeps
// traveling but the entire strip swells and dims underneath it.
//
// Other breathing variants to try later (see docs/stage-1-notes.md):
//   B - background glow only (comet stays full-range, only the "empty"
//       LEDs breathe)
//   C - additive second layer (breathing brightness added on top of
//       wave brightness, capped at max)

// Select board: Adafruit Feather ESP32-S3 No PSRAM

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 42
#define DATA_PIN A5
#define GAMMA 2.8

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

const float tailWidth = 4.0;  // how many LEDs the trailing glow fades out over, behind the leader
const float leadWidth = 2.0;  // how sharp the leading edge itself is
const float pulseSpeed = 0.3;
const float offsetRange = 4.0; // how far a LED's effective position can shift, in LED-widths (keep below tailWidth or the tail shape breaks down)

// Breathing (variant A: global brightness envelope)
const unsigned long breathPeriodMs = 6000; // one full dim-and-brighten cycle
const float breathMin = 0.3; // never dims below this, so the comet doesn't disappear entirely

float pulsePos = 0;
float posOffset[NUM_LEDS];

void setup() {
  strip.begin();
  strip.setBrightness(150); // cap max brightness, keeps current draw comfortably under supply limit
  strip.show();
  randomSeed(analogRead(A0)); // A0 is unused/floating, gives a different seed each run

  for (int i = 0; i < NUM_LEDS; i++) {
    posOffset[i] = random(-100, 100) / 100.0 * offsetRange;
  }
}

void loop() {
  // 0..1 phase through the breathing cycle, then mapped through a cosine
  // so it eases in/out smoothly rather than moving at constant speed.
  float breathPhase = fmodf(millis(), (float)breathPeriodMs) / (float)breathPeriodMs;
  float breathEnvelope = breathMin + (1.0 - breathMin) * (0.5 * (1.0 - cos(2.0 * PI * breathPhase)));

  for (int i = 0; i < NUM_LEDS; i++) {
    float pos = fmodf(i + posOffset[i] + NUM_LEDS, (float)NUM_LEDS); // keep pos in [0, NUM_LEDS) even when the offset pushes it negative

    float signedDist = pulsePos - pos; // positive = behind the leader (trailing tail), negative = ahead (not yet reached)
    if (signedDist > NUM_LEDS / 2.0) signedDist -= NUM_LEDS;
    if (signedDist < -NUM_LEDS / 2.0) signedDist += NUM_LEDS;

    float waveBrightness;
    if (signedDist >= 0) {
      waveBrightness = 1.0 - (signedDist / tailWidth);
    } else {
      waveBrightness = 1.0 - (-signedDist / leadWidth);
    }
    if (waveBrightness < 0) waveBrightness = 0;

    // Combine in linear space before gamma-correcting once, so the fade
    // stays smooth instead of compounding two separate gamma curves.
    float combined = waveBrightness * breathEnvelope;
    uint8_t level = pow(combined, GAMMA) * 255;

    strip.setPixelColor(i, strip.Color(level, level, level));
  }
  strip.show();

  pulsePos += pulseSpeed;
  if (pulsePos >= NUM_LEDS) pulsePos -= NUM_LEDS;

  delay(40);
}
