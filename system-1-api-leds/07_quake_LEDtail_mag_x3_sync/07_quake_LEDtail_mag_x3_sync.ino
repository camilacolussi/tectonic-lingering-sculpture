/*
  05_quake_LEDtail

  Combines 01_led_test's LED wave-tail pattern with 04_new-quake's USGS
  earthquake detection: the wave tail runs continuously, but when a new
  earthquake is detected, the whole strip snaps to solid white for 5
  seconds, then resumes the wave tail pattern from where it left off.

  Checks USGS every minute over eduroam (WPA2-Enterprise) — see
  02_ESP32_eduroam for the connection method on its own.

  LED pin 12: blinks while not connected, steady on once connected.
  Connection is only attempted in setup() — no automatic reconnect if it
  drops later.

IMPORTANT!!!!!!
- Select board : Adafruit ESP32 Feather
- Select Upload speed: Tools → Upload Speed → 115200
- Add WIFI credentials

COB EXPERIMENT:
const float tailWidth = 6.0; //4.0 original
const float leadWidth = 4.0; // 2.0 original
const float pulseSpeed = 0.3;
const float offsetRange = 2.0; //4.0 original

*/

#include <Adafruit_NeoPixel.h>
#include <WiFiEnterprise.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ---- LED strip / wave-tail pattern (from 01_led_test) ----

#define NUM_LEDS 41
#define DATA_PIN A5
#define GAMMA 2.8

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

const float tailWidth = 4.0;
const float leadWidth = 2.0;
const float pulseSpeed = 0.3;
const float offsetRange = 4.0;

float pulsePos = 0;
float posOffset[NUM_LEDS];

// ---- Quake flash override ----
// Flash duration scales with the detected quake's magnitude: MIN_MAGNITUDE
// (4.5) holds for QUAKE_FLASH_DURATION_MIN_MS, magnitude 9.0 holds for
// QUAKE_FLASH_DURATION_MAX_MS, and everything in between is a linear
// interpolation between those two points (computed in quakeFlashDurationForMagnitude()).

const unsigned long QUAKE_FLASH_DURATION_MIN_MS = 5000;    // at MIN_MAGNITUDE (4.5)
const unsigned long QUAKE_FLASH_DURATION_MAX_MS = 180000;  // at magnitude 9.0 (3 minutes)
const float QUAKE_FLASH_DURATION_MAX_MAGNITUDE = 9.5;

bool quakeFlashActive = false;
unsigned long quakeFlashStartTime = 0;
unsigned long quakeFlashDurationMs = QUAKE_FLASH_DURATION_MIN_MS; // set per-quake, see handleResponse()

// ---- WiFi status LED ----
// Blinks while not connected, steady on once connected. Connection is only
// ever attempted in setup() — if it drops later, the LED blinks to show
// that, but nothing tries to reconnect automatically.

const int WIFI_LED_PIN = 12;
const unsigned long BLINK_INTERVAL_MS = 200;
bool blinkState = false;
unsigned long lastBlinkToggleTime = 0;

// ---- eduroam / WiFiEnterprise ----

const char* EDUROAM_SSID = "eduroam";

// Fill these in by hand right before uploading, then blank them back out to
// "" before saving — never commit real values here.
const char* eduroamUsername = "";
const char* eduroamPassword = "";

// ---- USGS earthquake API ----

const float MIN_MAGNITUDE = 4.5;
const char* API_HOST = "earthquake.usgs.gov";

String buildRequestPath() {
  String path = "/fdsnws/event/1/query?format=text&limit=1&orderby=time&minmagnitude=";
  path += String(MIN_MAGNITUDE, 1);
  return path;
}

const unsigned long FETCH_INTERVAL_MS = 60000; // 1 minute
unsigned long lastFetchTime = 0;

// Tracks the most recently seen earthquake's unique ID, so a repeat fetch
// of the same event can be told apart from an actually new one.
String lastEventId = "";

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  strip.begin();
  strip.setBrightness(150);
  strip.show();
  randomSeed(analogRead(A0));

  for (int i = 0; i < NUM_LEDS; i++) {
    posOffset[i] = random(-100, 100) / 100.0 * offsetRange;
  }

  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);

  Serial.println("Connecting to eduroam (WPA2-Enterprise)...");
  if (WiFiEnterprise.begin(EDUROAM_SSID, eduroamUsername, eduroamPassword, true)) {
    Serial.print("Connected, IP: ");
    Serial.println(WiFiEnterprise.localIP());
    digitalWrite(WIFI_LED_PIN, HIGH); // steady on while connected
  } else {
    Serial.println("Initial connection failed — no automatic retry, LED will blink.");
  }

  checkForNewQuake();
  lastFetchTime = millis();
}

void loop() {
  if (millis() - lastFetchTime >= FETCH_INTERVAL_MS) {
    checkForNewQuake();
    lastFetchTime = millis();
  }

  // Placed before the quakeFlashActive early-return below, so the WiFi LED
  // keeps updating even while the strip is mid-flash.
  if (WiFiEnterprise.isConnected()) {
    digitalWrite(WIFI_LED_PIN, HIGH); // steady on
  } else if (millis() - lastBlinkToggleTime >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    digitalWrite(WIFI_LED_PIN, blinkState ? HIGH : LOW);
    lastBlinkToggleTime = millis();
  }

  if (quakeFlashActive) {
    if (millis() - quakeFlashStartTime >= quakeFlashDurationMs) {
      quakeFlashActive = false; // falls through to resume the wave this same loop
    } else {
      delay(40); // same per-iteration pacing as the wave pattern, just idling
      return;
    }
  }

  renderWaveFrame();
  delay(40);
}

void renderWaveFrame() {
  for (int i = 0; i < NUM_LEDS; i++) {
    float pos = fmodf(i + posOffset[i] + NUM_LEDS, (float)NUM_LEDS);

    float signedDist = pulsePos - pos;
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
}

// Maps a quake's magnitude to how long the flash should hold, linearly
// interpolating between (MIN_MAGNITUDE -> QUAKE_FLASH_DURATION_MIN_MS) and
// (QUAKE_FLASH_DURATION_MAX_MAGNITUDE -> QUAKE_FLASH_DURATION_MAX_MS).
// Magnitude is clamped to that range first, so anything at or below
// MIN_MAGNITUDE gets the minimum duration and anything at or above 9.0 gets
// the maximum — no extrapolating past either end.
unsigned long quakeFlashDurationForMagnitude(float magnitude) {
  float clamped = constrain(magnitude, MIN_MAGNITUDE, QUAKE_FLASH_DURATION_MAX_MAGNITUDE);
  float t = (clamped - MIN_MAGNITUDE) / (QUAKE_FLASH_DURATION_MAX_MAGNITUDE - MIN_MAGNITUDE);
  return QUAKE_FLASH_DURATION_MIN_MS + (unsigned long)(t * (QUAKE_FLASH_DURATION_MAX_MS - QUAKE_FLASH_DURATION_MIN_MS));
}

// ---- USGS fetch / new-quake detection (from 04_new-quake) ----

void checkForNewQuake() {
  if (!WiFiEnterprise.isConnected()) {
    // No reconnect attempt here — connection is only ever established in
    // setup(). If it's dropped, skip this fetch and let the LED show it.
    Serial.println("Not connected to eduroam, skipping this fetch.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://") + API_HOST + buildRequestPath();
  http.begin(client, url);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP request failed, code: ");
    Serial.println(httpCode);
    http.end();
    return;
  }

  String response = http.getString();
  http.end();

  handleResponse(response);
}

void handleResponse(const String& response) {
  // Field 0 is EventID, field 10 is Magnitude — see 04_new-quake for the
  // full field layout of a USGS text-format response.

  int firstDataLineStart = response.indexOf('\n');
  if (firstDataLineStart == -1) {
    Serial.println("Unexpected response (no header/data split found).");
    return;
  }

  String dataLine = response.substring(firstDataLineStart + 1);
  dataLine.trim();
  if (dataLine.length() == 0) {
    Serial.println("No earthquake found above threshold in this fetch.");
    return;
  }

  String eventId;
  String magnitudeStr;
  int fieldIndex = 0;
  int searchStart = 0;
  while (fieldIndex <= 10) {
    int nextPipe = dataLine.indexOf('|', searchStart);
    String field = (nextPipe == -1)
      ? dataLine.substring(searchStart)
      : dataLine.substring(searchStart, nextPipe);

    if (fieldIndex == 0) eventId = field;
    if (fieldIndex == 10) magnitudeStr = field;

    if (nextPipe == -1) break;
    searchStart = nextPipe + 1;
    fieldIndex++;
  }

  Serial.print("Magnitude: ");
  Serial.println(magnitudeStr);

  if (eventId == lastEventId) {
    return; // already reported this one
  }
  lastEventId = eventId;

  quakeFlashDurationMs = quakeFlashDurationForMagnitude(magnitudeStr.toFloat());

  Serial.print("New earthquake detected! Flashing strip for ");
  Serial.print(quakeFlashDurationMs / 1000.0);
  Serial.println("s.");

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();

  quakeFlashActive = true;
  quakeFlashStartTime = millis();
}
