// Touch test: shows raw touchRead() values on the OLED so we can see the
// actual touched vs. untouched range before choosing a threshold.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C // standard default I2C address for 0.96" SSD1306 modules

#define ThresLED 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int touchPin = A4; // GPIO14 / touch channel T14
const long touchThreshold = 100000; // untouched ~32k, touched ~228k+ (see stage-1-notes.md)

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(ThresLED, OUTPUT);

  // This board gates power to its I2C bus via I2C_POWER (GPIO7) - must be
  // driven HIGH or the OLED stays silent even when wired correctly.
  pinMode(I2C_POWER, OUTPUT);
  digitalWrite(I2C_POWER, HIGH);
  delay(50);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 not found - check wiring/address");
    while (true) delay(10);
  }

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  int value = touchRead(touchPin);
  // ESP32-S3 touch values increase when touched (opposite of the original ESP32)
  Serial.println(value);

  digitalWrite(ThresLED, value > touchThreshold ? HIGH : LOW);

  display.clearDisplay();
  display.setCursor(0, 20);
  display.print(value);
  display.display();

  delay(100);
}
