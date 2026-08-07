/*
  WiFiEnterprise Basic Connection Example
  
  This example demonstrates how to connect to a WPA2-Enterprise network
  using the WiFiEnterprise library with just SSID, username, and password.

  LED pin 12: blink if WiFi connection is unsuccessful; turn on if sucessfull. 
  However, it will turn off while looking to connect.

IMPORTANT:
- Select board : Adafruit ESP32 Feather
- Select Upload speed: Tools → Upload Speed → 115200
*/

#include <WiFiEnterprise.h>

const char* ssid = "eduroam";

const char* username = ""; //KU email
const char* password = "";

// Status LED: blinks while not connected, steady on once connected.
const int WIFI_LED_PIN = 12;
const unsigned long BLINK_INTERVAL_MS = 200; // toggle every 200ms while disconnected
bool blinkState = false;
unsigned long lastBlinkToggleTime = 0;

const unsigned long RECONNECT_INTERVAL_MS = 10000; // try again this often while disconnected
unsigned long lastConnectAttemptTime = 0;

const unsigned long STATUS_PRINT_INTERVAL_MS = 10000; // how often to log "still connected"
unsigned long lastStatusPrintTime = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);

  Serial.println("WiFiEnterprise Basic Connection Example");
  Serial.println("=====================================");

  // Connect to WPA2-Enterprise network with debug enabled
  Serial.println("Connecting to WPA2-Enterprise network...");

  if (WiFiEnterprise.begin(ssid, username, password, true)) {
    Serial.println("\n✅ Connection successful!");
    Serial.print("IP Address: ");
    Serial.println(WiFiEnterprise.localIP());
    Serial.print("Status: ");
    Serial.println(WiFiEnterprise.status());
    digitalWrite(WIFI_LED_PIN, HIGH); // steady on while connected
  } else {
    Serial.println("\n❌ Connection failed!");
    Serial.println("Please check your credentials and network settings.");
  }

  lastConnectAttemptTime = millis();
}

void loop() {
  if (WiFiEnterprise.isConnected()) {
    digitalWrite(WIFI_LED_PIN, HIGH); // steady on

    if (millis() - lastStatusPrintTime >= STATUS_PRINT_INTERVAL_MS) {
      Serial.println("📶 Still connected to enterprise network");
      Serial.print("IP: ");
      Serial.println(WiFiEnterprise.localIP());
      lastStatusPrintTime = millis();
    }
    return;
  }

  // Not connected: toggle the LED every BLINK_INTERVAL_MS using millis()
  // instead of a delay()-based blink loop, so it blinks continuously and
  // loop() stays free to do other checks in between — the same reasoning
  // as the millis()-based timers in the other sketches in this project.
  if (millis() - lastBlinkToggleTime >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    digitalWrite(WIFI_LED_PIN, blinkState ? HIGH : LOW);
    lastBlinkToggleTime = millis();
  }

  // Attempt to (re)connect every RECONNECT_INTERVAL_MS. Note this attempt
  // itself is still a blocking call (see below) — the LED will freeze
  // wherever it was for the few seconds the handshake takes. The blinking
  // above happens continuously in the gaps *between* attempts, which is as
  // close to "always blinks while disconnected" as this library allows.
  if (millis() - lastConnectAttemptTime >= RECONNECT_INTERVAL_MS) {
    Serial.println("❌ Not connected — attempting to (re)connect...");
    digitalWrite(WIFI_LED_PIN, LOW); // off for the duration of the blocking attempt below
    if (WiFiEnterprise.begin(ssid, username, password, true)) {
      Serial.println("✅ Reconnected successfully!");
      digitalWrite(WIFI_LED_PIN, HIGH);
    } else {
      Serial.println("❌ Reconnection failed!");
    }
    lastConnectAttemptTime = millis();
  }
}