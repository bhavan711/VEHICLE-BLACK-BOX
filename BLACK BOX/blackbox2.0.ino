#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>
#include <MPU6050_light.h>

// ==== Pin Definitions ====
#define SDA_OLED 21
#define SCL_OLED 22
#define SDA_MPU 25
#define SCL_MPU 26
#define LM35_PIN 34
#define SD_CS 5
#define GSM_RX 16
#define GSM_TX 17

// ==== OLED Setup ====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// ==== MPU6050 Setup ====
TwoWire I2CMPU = TwoWire(1);
MPU6050 mpu(I2CMPU);

// ==== GSM ====
HardwareSerial SerialGSM(2);

// ==== WiFi Credentials ====
const char* ssid = "OnePlus Nord CE4";
const char* password = "r9464grz";

// ==== ThingSpeak API Keys ====
const char* thingspeakAPIKey = "VEPNOYCBGCKK9LP4";
const char* secondAPIKey = "MFCRDDNMUZN7ANT5";
const char* server = "http://api.thingspeak.com/update";

// ==== Variables ====
float temperature = 0.0;
unsigned long lastUpload = 0;
bool crashDetected = false;
bool overSpeedLogged = false;
bool bumpDetected = false;

const float ACC_THRESHOLD = 2.0;
const float BUMP_THRESHOLD = 1.2;
const float SPEED_FAKE_THRESHOLD = 3.5;  // rough high-acceleration

void sendSMS(String message) {
  SerialGSM.println("AT+CMGF=1");
  delay(100);
  SerialGSM.println("AT+CMGS=\"+917538834864\"");
  delay(200);
  SerialGSM.print(message);
  delay(100);
  SerialGSM.write(26); // CTRL+Z
  delay(5000);
}

void setup() {
  Serial.begin(115200);

  // OLED Init
  Wire.begin(SDA_OLED, SCL_OLED);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Starting BlackBox...");
  display.display();

  // MPU Init
  I2CMPU.begin(SDA_MPU, SCL_MPU);
  mpu.begin();
  mpu.calcOffsets();

  // SD Card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card failed!");
  } else {
    Serial.println("SD Card OK");
  }

  // GSM Init
  SerialGSM.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);

  // WiFi Init
  WiFi.begin(ssid, password);
  display.println("Connecting WiFi...");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.println("WiFi Connected");
  display.display();
}

void loop() {
  mpu.update();

  temperature = analogRead(LM35_PIN) * (3.3 / 4095.0) * 100.0;

  float ax = mpu.getAccX();
  float ay = mpu.getAccY();
  float az = mpu.getAccZ();
  float gx = mpu.getGyroX();
  float gy = mpu.getGyroY();
  float gz = mpu.getGyroZ();

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("T:%.2f C\n", temperature);
  display.printf("AX:%.2f AY:%.2f\n", ax, ay);
  display.printf("AZ:%.2f\n", az);
  display.printf("GX:%.2f GY:%.2f\n", gx, gy);
  display.printf("GZ:%.2f\n", gz);
  display.display();

  // Calculate acceleration magnitude
  float accMagnitude = sqrt(ax * ax + ay * ay + az * az);
  
  // High Speed Detection
  if (accMagnitude > SPEED_FAKE_THRESHOLD && !overSpeedLogged) {
    Serial.println("HIGH SPEED DETECTED");
    File file = SD.open("/log.txt", FILE_APPEND);
    if (file) {
      file.println("HIGH SPEED DETECTED via accel");
      file.close();
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("HIGH SPEED!");
    display.setTextSize(1);
    display.display();
    overSpeedLogged = true;
  }
  if (accMagnitude < SPEED_FAKE_THRESHOLD - 0.5) {
    overSpeedLogged = false;
  }

  // Crash Detection (only if not in high speed condition)
  static float lastAX = 0, lastAY = 0, lastAZ = 0;
  bool isMoving = abs(ax - lastAX) > 0.2 || abs(ay - lastAY) > 0.2 || abs(az - lastAZ) > 0.2;

  if (!overSpeedLogged && (abs(ax) > ACC_THRESHOLD || abs(ay) > ACC_THRESHOLD || abs(az) > ACC_THRESHOLD) && isMoving) {
    if (!crashDetected) {
      crashDetected = true;
      Serial.println("CRASH DETECTED");

      String crashData = "CRASH! Temp:" + String(temperature) + ", AX:" + String(ax) + ", AY:" + String(ay) + ", AZ:" + String(az) +
                         ", GX:" + String(gx) + ", GY:" + String(gy) + ", GZ:" + String(gz);

      File file = SD.open("/crash.txt", FILE_APPEND);
      if (file) {
        file.println(crashData);
        file.close();
      }

      sendSMS(crashData);
    }
  } else {
    crashDetected = false;
  }

  // Bump Detection (only if not in high speed or crash condition)
  if (!crashDetected && !bumpDetected && !overSpeedLogged) {
    float da = abs(ax - lastAX) + abs(ay - lastAY) + abs(az - lastAZ);
    if (da > BUMP_THRESHOLD && da < ACC_THRESHOLD) {
      bumpDetected = true;
      Serial.println("BUMP DETECTED");
      File file = SD.open("/bump.txt", FILE_APPEND);
      if (file) {
        file.printf("BUMP: T:%.2f AX:%.2f AY:%.2f AZ:%.2f\n", temperature, ax, ay, az);
        file.close();
      }

      // OLED BUMP alert
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("BUMP!");
      display.setTextSize(1);
      display.display();
      delay(2000);
    }
  } else if (abs(ax - lastAX) < 0.1 && abs(ay - lastAY) < 0.1 && abs(az - lastAZ) < 0.1) {
    bumpDetected = false;
  }

  lastAX = ax;
  lastAY = ay;
  lastAZ = az;

  // Regular Logging
  File file = SD.open("/log.txt", FILE_APPEND);
  if (file) {
    file.printf("T:%.2f AX:%.2f AY:%.2f AZ:%.2f GX:%.2f GY:%.2f GZ:%.2f\n", temperature, ax, ay, az, gx, gy, gz);
    file.close();
  }

  // Upload to ThingSpeak every 15 sec
  if (millis() - lastUpload > 15000) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url1 = String(server) + "?api_key=" + thingspeakAPIKey +
                    "&field1=" + temperature + "&field2=" + ax + "&field3=" + ay +
                    "&field4=" + az + "&field5=" + gx + "&field6=" + gy + "&field7=" + gz;
      http.begin(url1);
      int httpCode1 = http.GET();
      if (httpCode1 > 0) Serial.println("Uploaded to ThingSpeak 1");
      else Serial.println("Upload Failed 1");
      http.end();

      String url2 = String(server) + "?api_key=" + secondAPIKey +
                    "&field1=" + temperature + "&field2=" + ax + "&field3=" + ay +
                    "&field4=" + az + "&field5=" + gx + "&field6=" + gy + "&field7=" + gz;
      http.begin(url2);
      int httpCode2 = http.GET();
      if (httpCode2 > 0) Serial.println("Uploaded to ThingSpeak 2");
      else Serial.println("Upload Failed 2");
      http.end();
    }
    lastUpload = millis();
  }

  delay(1000);
}
