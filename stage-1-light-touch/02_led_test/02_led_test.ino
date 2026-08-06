// LED motion test: wave pulse - a bright leader travels the strip with a
// soft trailing tail behind it (comet-style, not symmetric). No touch
// logic yet, this is just for testing/tuning the motion itself.
//
// White light, smooth over time - no per-frame flicker. Each LED has a
// small fixed position offset (chosen once at startup) so neighboring
// LEDs don't peak in strict index order as the leader passes - e.g. LED 4
// might catch the light slightly before LED 3. Each LED's own brightness
// still rises and falls perfectly smoothly.

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 42
#define DATA_PIN A5
#define GAMMA 2.8

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

const float tailWidth = 4.0;  // how many LEDs the trailing glow fades out over, behind the leader
const float leadWidth = 2.0;  // how sharp the leading edge itself is
const float pulseSpeed = 0.3;
const float offsetRange = 4.0; // how far a LED's effective position can shift, in LED-widths (keep below tailWidth or the tail shape breaks down)

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
    uint8_t level = pow(waveBrightness, GAMMA) * 255;

    strip.setPixelColor(i, strip.Color(level, level, level));
  }
  strip.show();

  pulsePos += pulseSpeed;
  if (pulsePos >= NUM_LEDS) pulsePos -= NUM_LEDS;

  delay(40);
}
