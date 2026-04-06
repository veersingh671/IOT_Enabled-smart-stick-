#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char* ssid = "********";     // Your SSID
const char* password = "********";  // Your PASSWORD

#define BOT_TOKEN "***********************"   // Change this to the your bot token
#define CHAT_ID "****************"            // Change this to the you chat ID

// Hardware Serial for GPS (UART1)
HardwareSerial gpsSerial(1);

// Pins
#define TRIG_PIN    5
#define ECHO_PIN    18
#define BUZZER_PIN  4          // Change if you use different pin

// Detection Settings
#define BUZZER_MIN_DISTANCE  25   // cm
#define BUZZER_MAX_DISTANCE  40   // cm
#define ALERT_THRESHOLD      20   // cm - critical for Telegram
#define HYSTERESIS           45   // cm
#define CONFIRMATION_TIME    2000 // ms (2 seconds)

// Ultrasonic Filtering Settings (for stable readings)
#define NUM_SAMPLES          7    // Number of readings for median filter
#define FILTER_DELAY         30   // ms delay between samples



TinyGPSPlus gps;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

float lat = 0.0, lng = 0.0; 
bool gpsHasFix = false;
bool alertSent = false;
unsigned long detectionStart = 0;
bool objectDetected = false;

unsigned long lastPrint = 0;
unsigned long lastWiFiCheck = 0;

// For median filter
float readings[NUM_SAMPLES];
int readIndex = 0;

// SETUP
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);   

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("ESP32 GPS + Stable Ultrasonic + Buzzer + Telegram");
  Serial.println("Ultrasonic sensor optimized with median filter");

  // WiFi Connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  delay(2000);
}


void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    gpsHasFix = true;
  }

  float distance = getFilteredDistance();   // Now using stable filtered reading

  // BUZZER LOGIC
  if (distance >= BUZZER_MIN_DISTANCE && distance <= BUZZER_MAX_DISTANCE) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Print status every 3 seconds
  if (millis() - lastPrint > 3000) {
    lastPrint = millis();
    
    Serial.println("----------------------------------------");
    Serial.printf("Satellites : %d\n", gps.satellites.isValid() ? gps.satellites.value() : 0);
    
    if (gpsHasFix) {
      Serial.printf("GPS Fix: %.6f, %.6f\n", lat, lng);
    } else {
      Serial.println(" No GPS Fix yet...");
    }
    
    Serial.printf("Distance (Filtered): %.2f cm\n", distance);
    Serial.println("");
  }

  // TELEGRAM ALERT LOGIC
  if (distance < ALERT_THRESHOLD) {
    if (!objectDetected) {
      detectionStart = millis();
      objectDetected = true;
      Serial.println("Critical object (<40cm) - Starting 2s confirmation");
    }

    if ((millis() - detectionStart >= CONFIRMATION_TIME) && !alertSent && gpsHasFix) {
      sendTelegramAlert();
      alertSent = true;
    }
  } 
  else {
    if (distance > HYSTERESIS) {
      objectDetected = false;
      alertSent = false;
    }
  }

  // WiFi check
  if (millis() - lastWiFiCheck > 30000) {
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Reconnecting...");
      WiFi.reconnect();
    }
  }

  delay(200);  
}

float getFilteredDistance() {
  for (int i = 0; i < NUM_SAMPLES; i++) {
    readings[i] = singlePing();
    delay(FILTER_DELAY);              
  }
  sortArray(readings, NUM_SAMPLES);
  float median = readings[NUM_SAMPLES / 2];
  static float smoothed = median;
  smoothed = smoothed * 0.7 + median * 0.3;
  return smoothed;
}

float singlePing() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(4);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(12);        // Slightly longer trigger pulse
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5 meters)

  if (duration == 0) return 999.0;   // No echo / timeout

  return (duration * 0.0343) / 2.0;  // Speed of sound calculation
}

// Simple bubble sort for median
void sortArray(float arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

//  TELEGRAM ALERT 
void sendTelegramAlert() {
  Serial.println("Sending Telegram Alert...");
  String message = "*ALERT!* Critical Object Detected (<20 cm)!\n\n";
  message += "*Location:*\n";
  message += "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lng, 6) + "\n\n";
  message += "Time: " + String(millis() / 1000) + "s\n";
  message += "Satellites: " + String(gps.satellites.value());
  bool success = bot.sendMessage(CHAT_ID, message, "Markdown");

  if (success) {
    Serial.println("Telegram sent successfully!");
  } else {
    Serial.println("Failed to send Telegram message");
  }
}
