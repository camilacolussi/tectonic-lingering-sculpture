/*
  03_API_test

  Fetches the most recent earthquake (magnitude 4.5+) from the USGS
  earthquake API and prints its magnitude to Serial once a minute.
  Connects over eduroam (WPA2-Enterprise) instead of a plain home WiFi
  network — see 02_ESP32_eduroam for the connection method on its own.

  LED pin 12: blinks while not connected, steady on once connected.
  Connection is only attempted in setup() — no automatic reconnect if it
  drops later.

IMPORTANT:
- Select board : Adafruit ESP32 Feather
- Select Upload speed: Tools → Upload Speed → 115200
  
*/

#include <WiFiEnterprise.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* EDUROAM_SSID = "eduroam";
const char* eduroamUsername = "ku72395@kingston.ac.uk"; // KU email
const char* eduroamPassword = "Camiking1.!";

// Status LED: blinks while not connected, steady on once connected.
// Connection is only ever attempted in setup() here — if it drops later,
// the LED blinks to show that, but nothing tries to reconnect automatically.
const int WIFI_LED_PIN = 12;
const unsigned long BLINK_INTERVAL_MS = 200;
bool blinkState = false;
unsigned long lastBlinkToggleTime = 0;

// Only earthquakes at or above this magnitude count as "the event" — keeps
// the result from being dominated by constant, barely-felt micro-quakes.
const float MIN_MAGNITUDE = 4.5;

// USGS FDSN Event Web Service, plain-text format:
// - format=text -> pipe-delimited plain text, not JSON. No JSON parsing
//   library needed, and the response is a fraction of the size of the
//   equivalent GeoJSON feed.
// - limit=1&orderby=time -> USGS returns just the single most recent
//   matching event, instead of us downloading a whole feed and picking one
//   out ourselves.
// - minmagnitude filters server-side too, before anything is sent to us.
const char* API_HOST = "earthquake.usgs.gov";

String buildRequestPath() {
  String path = "/fdsnws/event/1/query?format=text&limit=1&orderby=time&minmagnitude=";
  path += String(MIN_MAGNITUDE, 1);
  return path;
}

const unsigned long FETCH_INTERVAL_MS = 60000; // 1 minute
unsigned long lastFetchTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

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

  fetchLatestMagnitude();
  lastFetchTime = millis();
}

void loop() {
  // millis()-based timer instead of delay(60000): this keeps loop() free to
  // do other work on every pass. Doesn't matter yet, but it will once this
  // merges with the LED animation, which needs to keep rendering frames
  // continuously and can't tolerate the board freezing for a network call.
  if (millis() - lastFetchTime >= FETCH_INTERVAL_MS) {
    fetchLatestMagnitude();
    lastFetchTime = millis();
  }

  if (WiFiEnterprise.isConnected()) {
    digitalWrite(WIFI_LED_PIN, HIGH); // steady on
  } else if (millis() - lastBlinkToggleTime >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    digitalWrite(WIFI_LED_PIN, blinkState ? HIGH : LOW);
    lastBlinkToggleTime = millis();
  }
}

void fetchLatestMagnitude() {
  if (!WiFiEnterprise.isConnected()) {
    // No reconnect attempt here — connection is only ever established in
    // setup(). If it's dropped, skip this fetch and let the LED show it.
    Serial.println("Not connected to eduroam, skipping this fetch.");
    return;
  }

  WiFiClientSecure client;
  // Skips verifying USGS's TLS certificate. Proper verification needs a
  // root CA certificate stored on the board (and kept up to date as certs
  // rotate) — real maintenance overhead for read-only public data with
  // nothing sensitive in the request. Standard tradeoff for hobby ESP32
  // projects; noting it rather than hiding it.
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
  http.end(); // free the connection promptly

  parseAndPrintMagnitude(response);
}

void parseAndPrintMagnitude(const String& response) {
  // A matching response looks like:
  // #EventID|Time|Latitude|Longitude|Depth|Author|Catalog|Contributor|ContributorID|MagType|Magnitude|MagAuthor|EventLocationName
  // us7000abcd|2026-08-07T12:34:56.000Z|34.5|-118.2|10.0|us|us|us|us7000abcd|mb|4.5|us|10km SSW of Somewhere
  // Field 10 (0-indexed) is Magnitude.

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

  String magnitudeStr;
  int fieldIndex = 0;
  int searchStart = 0;
  while (fieldIndex <= 10) {
    int nextPipe = dataLine.indexOf('|', searchStart);
    String field = (nextPipe == -1)
      ? dataLine.substring(searchStart)
      : dataLine.substring(searchStart, nextPipe);

    if (fieldIndex == 10) magnitudeStr = field;

    if (nextPipe == -1) break;
    searchStart = nextPipe + 1;
    fieldIndex++;
  }

  Serial.print("Magnitude: ");
  Serial.println(magnitudeStr);
}
