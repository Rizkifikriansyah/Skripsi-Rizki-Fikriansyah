#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>

// ========== WIFI ==========
const char* ssid = "Kiki S.Kom";
const char* password = "22222222";

// ========== TELEGRAM ==========
String BOTtoken = "8733038130:AAE-INunOp97XEmmbhg661vw7vQfqmNgmV8"; 
String CHAT_ID  = "8518442862";

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

// ========== GPS ==========
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

#define GPS_RX 16
#define GPS_TX 17

// ========== LED ==========
#define FLASH_LED_PIN 4

// ========== VARIABEL ==========
unsigned long lastTimeBotRan = 0;
int botRequestDelay = 1000;

unsigned long lastGpsPrint = 0;
const unsigned long gpsPrintInterval = 3000;

bool gpsDetected = false;
unsigned long lastSerialCheck = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===== ESP32 GPS + TELEGRAM =====");

  // LED
  pinMode(FLASH_LED_PIN, OUTPUT);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(3000);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi Failed");
  }

  // Telegram
  clientTCP.setInsecure();

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS Ready");

  Serial.println("System Ready 🚀");
}

// ================= LOOP =================
void loop() {

  // Baca GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
    gpsDetected = true;
  }

  // Warning jika GPS tidak terbaca
  if (millis() > 10000 && gps.charsProcessed() < 10 && !gpsDetected) {
    Serial.println("⚠️ GPS tidak terdeteksi!");
    gpsDetected = true;
  }

  // Print GPS tiap 3 detik
  if (millis() - lastGpsPrint > gpsPrintInterval) {
    printGpsInfo();
    lastGpsPrint = millis();
  }

  // Telegram
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastTimeBotRan > botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }

      lastTimeBotRan = millis();
    }
  }

  delay(10);
}

// ================= GPS INFO =================
void printGpsInfo() {
  Serial.println("\n=== GPS INFO ===");

  if (!gps.location.isValid()) {
    Serial.println("Menunggu GPS fix...");
    return;
  }

  Serial.print("Lat: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Lng: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Satelit: ");
  Serial.println(gps.satellites.value());
}

// ================= TELEGRAM =================
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized", "");
      continue;
    }

    if (text == "/start") {
      bot.sendMessage(CHAT_ID,
        "🤖 ESP32 GPS Bot\n\n"
        "/location - Lokasi\n"
        "/status - Status",
        "");
    }

    else if (text == "/location") {
      sendLocationToTelegram();
    }

    else if (text == "/status") {
      String msg = "WiFi: ";
      msg += (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

// ================= KIRIM LOKASI =================
void sendLocationToTelegram() {

  if (!gps.location.isValid()) {
    bot.sendMessage(CHAT_ID, "❌ GPS belum dapat lokasi", "");
    return;
  }

  String latStr = String(gps.location.lat(), 6);
  String lngStr = String(gps.location.lng(), 6);

  String msg = "📍 Lokasi Sapi\n";
  msg += "Lat: " + latStr + "\n";
  msg += "Lng: " + lngStr + "\n";

  bot.sendMessage(CHAT_ID, msg, "");

  String mapsLink = "https://www.google.com/maps?q=" + latStr + "," + lngStr;

  bot.sendMessage(CHAT_ID, mapsLink, "");

  Serial.println("Lokasi terkirim");
}