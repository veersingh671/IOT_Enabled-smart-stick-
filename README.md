
# ESP32 GPS + Ultrasonic Alert System (Telegram)

 Overview
This project is a real-time IoT-based alert system built using the ESP32, NEO-6M GPS module, and an ultrasonic sensor.
It detects nearby objects and sends a Telegram alert with live GPS location when a critical threshold is reached. The system is designed with noise filtering and smart detection logic for reliable real-world performance.

# Features
1. Real-time GPS location tracking
2. Telegram alert with Google Maps link
3.  Ultrasonic distance measurement with filtering
4.  Buzzer warning for nearby objects
5.  Smart detection (confirmation delay + hysteresis)
6.  Auto WiFi reconnect
7.  Optimized and stable for deployment


 # System Workflow
1. Ultrasonic sensor continuously measures distance
2. If the object is detected below 20 cm for 2 seconds:
3. Detection is confirmed (reduces false triggers)
    . GPS coordinates are fetched
    . Telegram alert is sent with the location
4. Buzzer activates when object is within 25–40 cm range


# Hardware Requirements
1. ESP32
2. NEO-6M GPS Module
3. HC-SR04 Ultrasonic Sensor
4. Buzzer
5. Jumper wires
6. Stable power supply

# Pin Configuration
| Component | ESP32 GPIO |
| --------- | ---------- |
| GPS TX    | 16         |
| GPS RX    | 17         |
| TRIG      | 5          |
| ECHO      | 18         |
| BUZZER    | 4          |

# Telegram Setup
1. Open Telegram
2. Search for @BotFather
3. Create a bot:
    /newbot
4. Copy the Bot Token
5. Get your Chat ID using @userinfobot   


# Required Libraries
  Install via Arduino IDE:
. WiFi.h
. WiFiClientSecure.h
. UniversalTelegramBot
. TinyGPS++

# Configuration
  Update credentials in the code:
  
   const char* ssid = "YOUR_WIFI";
   const char* password = "YOUR_PASSWORD";
   #define BOT_TOKEN "YOUR_BOT_TOKEN"
   #define CHAT_ID "YOUR_CHAT_ID"

  # Detection Parameters
  | Parameter         | Value    | Description               |
| ----------------- | -------- | ------------------------- |
| ALERT_THRESHOLD   | 20 cm    | Telegram trigger distance |
| CONFIRMATION_TIME | 2000 ms  | Detection must persist    |
| HYSTERESIS        | 45 cm    | Reset threshold           |
| BUZZER RANGE      | 25–40 cm | Warning zone              |

# Advantages
1. No SIM card required
2. No SMS cost
3. Instant alerts
4. Clickable live location
5. Reliable filtered sensor readings

# Future Improvements
1. ESP32-CAM integration
2. Cloud dashboard (Firebase / MQTT)
3. Continuous tracking system
4. Battery monitoring
5. Telegram command control

# Author
chiranjeev Veer singh
Electronics & Embedded Engineer
