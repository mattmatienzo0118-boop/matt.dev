/*
  Environment Smartwatch
  ESP32-S3 + GC9A01 round TFT + BME280 + MQ-135 + TRIOE telemetry

  Required libraries:
  - Adafruit GFX Library
  - Adafruit GC9A01A
  - Adafruit BME280 Library
  - Adafruit Unified Sensor
  - Trioe
*/

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <Adafruit_BME280.h>
#include <Trioe.h>
#include <TRIOE_ROOT_CA.h>

// Wi-Fi
const char *WIFI_SSID = "BatStateU-DevOps";
const char *WIFI_PASSWORD = "Dev3l$06";

// TRIOE
const char *TELEMETRY_URL = "https://hub.trioe.dev/api/v1/devices/TRIOE-330E1E9DD26A/telemetry/";
const char *API_KEY = "thub_sfdLzwiaxnc-3y60pNPtKYZtegEePvTKl6DKn2PoEIQ";

TrioeClient trioe(TELEMETRY_URL, API_KEY);

constexpr unsigned long TRIOE_REPORT_INTERVAL_MS = 30000;
constexpr float TRIOE_CHANGE_THRESHOLD = 0.2f;

// NTP
const char *NTP_SERVER_1 = "time.google.com";
const char *NTP_SERVER_2 = "pool.ntp.org";
const char *NTP_SERVER_3 = "time.nist.gov";

const long UTC_OFFSET_SECONDS = 8L * 3600L;
const int DST_OFFSET_SECONDS = 0;

// ---------- Pins (all on the right header of the board) ----------
constexpr uint8_t MQ135_PIN = 4;    // D4  -> MQ135 AO (ADC1)

constexpr uint8_t BME_SDA = 1;      // D5  -> BME280 SDA
constexpr uint8_t BME_SCL = 2;      // D6  -> BME280 SCL

constexpr uint8_t TFT_RST  = 7;     // D7  -> LCD RST
constexpr uint8_t TFT_CS   = 8;     // D8  -> LCD CS
constexpr uint8_t TFT_DC   = 9;     // pin labeled "9" -> LCD DC
constexpr uint8_t TFT_MOSI = 10;    // D10 -> LCD SDA
constexpr uint8_t TFT_SCLK = 14;    // D14 -> LCD SCL

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 240;

// Timing
constexpr unsigned long SENSOR_INTERVAL = 1500;
constexpr unsigned long RENDER_INTERVAL = 1000;
constexpr unsigned long BOOT_SCREEN_MS = 5000;

// Clock size
constexpr uint8_t CLOCK_TIME_SIZE = 2;
constexpr uint8_t CLOCK_AMPM_SIZE = 1;
constexpr int CLOCK_TIME_Y = 7;
constexpr int CLOCK_DATE_Y = 30;

// MQ-135 settings
constexpr float MQ_RL = 10.0;
constexpr float MQ_RZERO_DEFAULT = 76.63;
constexpr float MQ_PARA = 116.6020682;
constexpr float MQ_PARB = 2.769034857;
constexpr float MQ_ADC_MAX = 4095.0;

constexpr float MQ_CORA = 0.00035;
constexpr float MQ_CORB = 0.02718;
constexpr float MQ_CORC = 1.39538;
constexpr float MQ_CORD = 0.0018;

constexpr int MQ_MIN_VALID_ADC = 5;
constexpr int MQ_MAX_VALID_ADC = 4090;
constexpr float MQ_MIN_VALID_RS = 0.3;
constexpr float MQ_PPM_CLAMP_MAX = 5000.0;

float mqRZero = MQ_RZERO_DEFAULT;

// Devices
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BME280 bme;

// State
bool bmeReady = false;

unsigned long lastSensorRead = 0;
unsigned long lastRender = 0;
unsigned long lastTrioeSend = 0;

float temperatureC = NAN;
float humidity = NAN;
float pressureHpa = NAN;
float co2Estimate = NAN;

// Colors
uint16_t INK;
uint16_t MUTED;
uint16_t PANEL;
uint16_t ACCENT;
uint16_t GOOD;
uint16_t FAIR;
uint16_t POOR;
uint16_t DANGER;
uint16_t BOOT_GRAY;
uint16_t TRIOE_BLUE;

char lastHeaderKey[40] = "";
char lastEnvKey[64] = "";

// Layout
constexpr int CARD_W = 98;
constexpr int CARD_H = 62;

constexpr int CARD_TL_X = 18;
constexpr int CARD_TR_X = 124;
constexpr int CARD_TOP_Y = 57;
constexpr int CARD_BOT_Y = 127;

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

const char *getAirLabel(float ppm) {
  if (isnan(ppm)) return "SENSOR CHECK";
  if (ppm <= 600) return "FRESH AIR";
  if (ppm <= 900) return "FAIR";
  if (ppm <= 1400) return "VENTILATE";
  return "POOR AIR";
}

uint16_t getAirColor(float ppm) {
  if (isnan(ppm)) return MUTED;
  if (ppm <= 600) return GOOD;
  if (ppm <= 900) return FAIR;
  if (ppm <= 1400) return POOR;
  return DANGER;
}

float getStableMQAdc() {
  const int samples = 9;
  int values[samples];

  for (int i = 0; i < samples; i++) {
    values[i] = analogRead(MQ135_PIN);
    delay(3);
  }

  for (int i = 0; i < samples - 1; i++) {
    for (int j = i + 1; j < samples; j++) {
      if (values[j] < values[i]) {
        int temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
      }
    }
  }

  return values[samples / 2];
}

float getRawResistance(float adc) {
  if (adc <= MQ_MIN_VALID_ADC || adc >= MQ_MAX_VALID_ADC) {
    return NAN;
  }

  float rs = ((MQ_ADC_MAX * MQ_RL) / adc) - MQ_RL;

  if (rs < MQ_MIN_VALID_RS) return NAN;

  return rs;
}

float getCorrectionFactor(float temperature, float humidityValue) {
  return MQ_CORA * temperature * temperature
    - MQ_CORB * temperature
    + MQ_CORC
    - (humidityValue - 33.0F) * MQ_CORD;
}

float getCorrectedPPM(float temperature, float humidityValue) {
  float adc = getStableMQAdc();
  float rs = getRawResistance(adc);

  if (isnan(rs)) return NAN;

  float correctionFactor = getCorrectionFactor(temperature, humidityValue);

  if (correctionFactor <= 0) {
    correctionFactor = 1.0F;
  }

  float correctedRs = rs / correctionFactor;

  float ppm = MQ_PARA * pow(
    correctedRs / mqRZero,
    -MQ_PARB
  );

  if (!isfinite(ppm) || ppm < 0 || ppm > MQ_PPM_CLAMP_MAX) {
    return NAN;
  }

  return ppm;
}

float calibrateMQ135() {
  const int samples = 25;
  float values[samples];
  int validCount = 0;

  for (int i = 0; i < samples; i++) {
    float adc = getStableMQAdc();
    float rs = getRawResistance(adc);

    if (!isnan(rs)) {
      values[validCount++] = rs;
    }

    delay(150);
  }

  if (validCount == 0) return NAN;

  for (int i = 0; i < validCount - 1; i++) {
    for (int j = i + 1; j < validCount; j++) {
      if (values[j] < values[i]) {
        float temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
      }
    }
  }

  float medianRs = values[validCount / 2];

  float calculatedRZero = medianRs /
    pow(400.0F / MQ_PARA, 1.0F / -MQ_PARB);

  if (calculatedRZero <= 0 || !isfinite(calculatedRZero)) {
    return NAN;
  }

  return calculatedRZero;
}

void centered(const char *text, int y, uint8_t size, uint16_t color) {
  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  tft.setCursor((SCREEN_W - w) / 2, y);
  tft.print(text);
}

void roundedPanel(int x, int y, int w, int h) {
  tft.fillRoundRect(x, y, w, h, 16, PANEL);
}

void drawTrioeLogo(int cx, int cy, int radius) {
  int topX = cx;
  int topY = cy - radius;
  int rightX = cx + radius;
  int rightY = cy;
  int bottomX = cx;
  int bottomY = cy + radius;
  int leftX = cx - radius;
  int leftY = cy;

  for (int i = -2; i <= 2; i++) {
    tft.drawLine(topX + i, topY, rightX + i, rightY, TRIOE_BLUE);
    tft.drawLine(rightX, rightY + i, bottomX, bottomY + i, TRIOE_BLUE);
    tft.drawLine(bottomX + i, bottomY, leftX + i, leftY, TRIOE_BLUE);
    tft.drawLine(leftX, leftY + i, topX, topY + i, TRIOE_BLUE);

    tft.drawLine(leftX, cy + i, rightX, cy + i, TRIOE_BLUE);
    tft.drawLine(cx + i, cy, cx + i, bottomY, TRIOE_BLUE);
  }
}

void drawBootScreen() {
  tft.fillScreen(BOOT_GRAY);
  drawTrioeLogo(120, 83, 46);
  centered("TRIOE", 157, 3, TRIOE_BLUE);
  centered("environment watch", 193, 1, TRIOE_BLUE);
}

void showBootStatus(const char *message) {
  tft.fillRect(0, 200, SCREEN_W, 30, BOOT_GRAY);
  centered(message, 210, 1, TRIOE_BLUE);
}

void updateHeaderValues() {
  struct tm now;
  bool hasTime = getLocalTime(&now, 10);

  // HH:MM needs five characters plus the null terminator.
  char timeText[6] = "--:--";
  char meridiem[3] = "";
  char dateText[24] = "TIME NOT SYNCED";

  if (hasTime) {
    strftime(timeText, sizeof(timeText), "%I:%M", &now);
    strftime(meridiem, sizeof(meridiem), "%p", &now);
    strftime(dateText, sizeof(dateText), "%a, %b %d %Y", &now);
  }

  char headerKey[40];
  snprintf(
    headerKey,
    sizeof(headerKey),
    "%s|%s|%s",
    timeText,
    meridiem,
    dateText
  );

  if (strcmp(headerKey, lastHeaderKey) == 0) return;
  strcpy(lastHeaderKey, headerKey);

  tft.fillRect(0, 0, SCREEN_W, 50, GC9A01A_BLACK);

  int16_t x1, y1;
  uint16_t timeW, timeH;
  uint16_t ampmW, ampmH;

  tft.setTextSize(CLOCK_TIME_SIZE);
  tft.getTextBounds(timeText, 0, 0, &x1, &y1, &timeW, &timeH);

  tft.setTextSize(CLOCK_AMPM_SIZE);
  tft.getTextBounds(meridiem, 0, 0, &x1, &y1, &ampmW, &ampmH);

  int gap = hasTime ? 6 : 0;
  int totalWidth = timeW + gap + (hasTime ? ampmW : 0);
  int timeX = (SCREEN_W - totalWidth) / 2;

  tft.setTextSize(CLOCK_TIME_SIZE);
  tft.setTextColor(INK);
  tft.setCursor(timeX, CLOCK_TIME_Y);
  tft.print(timeText);

  if (hasTime) {
    tft.setTextSize(CLOCK_AMPM_SIZE);
    tft.setTextColor(TRIOE_BLUE);
    tft.setCursor(
      timeX + timeW + gap,
      CLOCK_TIME_Y + timeH - ampmH
    );
    tft.print(meridiem);
  }

  centered(dateText, CLOCK_DATE_Y, 1, MUTED);
  tft.drawFastHLine(55, 44, 130, PANEL);
}

void drawMetricCardStatic(
  int x,
  int y,
  int w,
  const char *caption,
  const char *unit
) {
  roundedPanel(x, y, w, CARD_H);

  tft.setTextColor(MUTED);
  tft.setTextSize(1);
  tft.setCursor(x + 12, y + 11);
  tft.print(caption);

  tft.setCursor(x + 12, y + 49);
  tft.print(unit);
}

void drawMetricValue(
  int x,
  int y,
  const char *value,
  uint16_t highlight
) {
  tft.fillRect(x + 10, y + 22, CARD_W - 18, 20, PANEL);

  tft.setTextColor(highlight);
  tft.setTextSize(2);
  tft.setCursor(x + 12, y + 29);
  tft.print(value);
}

void updateEnvironmentValues();

void drawMainPage() {
  tft.fillScreen(GC9A01A_BLACK);

  drawMetricCardStatic(CARD_TL_X, CARD_TOP_Y, CARD_W,
                       "TEMPERATURE", "degrees C");

  drawMetricCardStatic(CARD_TR_X, CARD_TOP_Y, CARD_W,
                       "HUMIDITY", "% RH");

  drawMetricCardStatic(CARD_TL_X, CARD_BOT_Y, CARD_W,
                       "PRESSURE", "hPa");

  drawMetricCardStatic(CARD_TR_X, CARD_BOT_Y, CARD_W,
                       "CO2 ESTIMATE", "ppm");

  lastHeaderKey[0] = '\0';
  lastEnvKey[0] = '\0';

  updateHeaderValues();
  updateEnvironmentValues();
}

void updateEnvironmentValues() {
  char temp[10];
  char hum[10];
  char press[10];
  char ppm[12];

  if (isnan(temperatureC)) strcpy(temp, "--");
  else snprintf(temp, sizeof(temp), "%.1f", temperatureC);

  if (isnan(humidity)) strcpy(hum, "--");
  else snprintf(hum, sizeof(hum), "%.0f", humidity);

  if (isnan(pressureHpa)) strcpy(press, "--");
  else snprintf(press, sizeof(press), "%.0f", pressureHpa);

  if (isnan(co2Estimate)) strcpy(ppm, "--");
  else snprintf(ppm, sizeof(ppm), "%.0f", co2Estimate);

  const char *statusLabel = getAirLabel(co2Estimate);
  uint16_t statusColor = getAirColor(co2Estimate);

  char envKey[64];

  snprintf(
    envKey,
    sizeof(envKey),
    "%s|%s|%s|%s|%s",
    temp,
    hum,
    press,
    ppm,
    statusLabel
  );

  if (strcmp(envKey, lastEnvKey) == 0) return;
  strcpy(lastEnvKey, envKey);

  drawMetricValue(CARD_TL_X, CARD_TOP_Y, temp, ACCENT);
  drawMetricValue(CARD_TR_X, CARD_TOP_Y, hum, rgb(86, 205, 235));
  drawMetricValue(CARD_TL_X, CARD_BOT_Y, press, rgb(185, 148, 255));
  drawMetricValue(CARD_TR_X, CARD_BOT_Y, ppm, statusColor);

  roundedPanel(45, 204, 150, 22);
  tft.fillRoundRect(45, 204, 150, 22, 11, statusColor);
  centered(statusLabel, 211, 1, GC9A01A_BLACK);
}

void readSensors() {
  if (bmeReady) {
    temperatureC = bme.readTemperature();
    humidity = bme.readHumidity();
    pressureHpa = bme.readPressure() / 100.0F;

    if (isnan(temperatureC) || isnan(humidity)) {
      temperatureC = NAN;
      humidity = NAN;
      pressureHpa = NAN;
    }
  }

  float correctionTemp = isnan(temperatureC) ? 25.0F : temperatureC;
  float correctionHum = isnan(humidity) ? 50.0F : humidity;

  float sample = getCorrectedPPM(correctionTemp, correctionHum);

  if (!isnan(sample) && isfinite(sample) && sample > 0) {
    co2Estimate = isnan(co2Estimate)
      ? sample
      : (0.15F * sample + 0.85F * co2Estimate);
  }
}

void sendTelemetry() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!isnan(temperatureC)) {
    trioe.addReading("temperature", temperatureC, "C");
  }

  if (!isnan(humidity)) {
    trioe.addReading("humidity", humidity, "%");
  }

  if (!isnan(pressureHpa)) {
    trioe.addReading("pressure", pressureHpa, "hPa");
  }

  if (!isnan(co2Estimate)) {
    trioe.addReading("co2_estimate", co2Estimate, "ppm");
  }

  trioe.addReading("air_status", getAirLabel(co2Estimate));
}

void connectTime() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  unsigned long started = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - started < 15000) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi connection failed. Time was not synced.");
    return;
  }

  Serial.println("\nWi-Fi connected.");

  configTime(
    UTC_OFFSET_SECONDS,
    DST_OFFSET_SECONDS,
    NTP_SERVER_1,
    NTP_SERVER_2,
    NTP_SERVER_3
  );

  struct tm timeInfo;

  for (int attempt = 0; attempt < 20; attempt++) {
    if (getLocalTime(&timeInfo, 1000)) {
      Serial.println("NTP time synced.");
      return;
    }

    delay(500);
  }

  Serial.println("NTP time synchronization failed.");
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  Wire.begin(BME_SDA, BME_SCL);
  bmeReady = bme.begin(0x76) || bme.begin(0x77);

  if (bmeReady) {
    Serial.println("BME280 initialized.");
  } else {
    Serial.println("BME280 not found. Check wiring on D5/D6.");
  }

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(0);

  INK = rgb(239, 244, 250);
  MUTED = rgb(132, 148, 169);
  PANEL = rgb(24, 34, 52);
  ACCENT = rgb(83, 230, 184);
  GOOD = rgb(83, 230, 184);
  FAIR = rgb(255, 203, 82);
  POOR = rgb(255, 143, 69);
  DANGER = rgb(255, 84, 112);
  BOOT_GRAY = rgb(218, 218, 218);
  TRIOE_BLUE = rgb(36, 58, 147);

  drawBootScreen();

  unsigned long bootStarted = millis();

  connectTime();

  if (WiFi.status() == WL_CONNECTED) {
    trioe.setCACert(TRIOE_ROOT_CA);
    trioe.setReportingInterval(TRIOE_REPORT_INTERVAL_MS);
    trioe.setChangeThreshold(TRIOE_CHANGE_THRESHOLD);

    if (!trioe.begin()) {
      Serial.println("TRIOE setup failed.");
    } else {
      Serial.println("TRIOE initialized.");
    }
  }

  showBootStatus("Calibrating sensor...");

  float calibrated = calibrateMQ135();

  if (!isnan(calibrated)) {
    mqRZero = calibrated;
    Serial.print("Calibration OK. RZERO = ");
    Serial.println(mqRZero);
  } else {
    Serial.println("Calibration failed. Using default RZERO.");
  }

  readSensors();

  unsigned long elapsed = millis() - bootStarted;

  if (elapsed < BOOT_SCREEN_MS) {
    delay(BOOT_SCREEN_MS - elapsed);
  }

  lastSensorRead = millis();
  lastRender = millis();
  lastTrioeSend = millis();

  drawMainPage();
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;
    readSensors();
  }

  if (now - lastRender >= RENDER_INTERVAL) {
    lastRender = now;
    updateHeaderValues();
    updateEnvironmentValues();
  }

  if (now - lastTrioeSend >= TRIOE_REPORT_INTERVAL_MS) {
    lastTrioeSend = now;
    sendTelemetry();
  }

  trioe.loop();
  delay(20);
}