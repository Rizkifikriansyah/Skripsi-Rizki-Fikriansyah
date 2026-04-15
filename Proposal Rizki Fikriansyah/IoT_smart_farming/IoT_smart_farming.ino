#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <TinyGPS++.h>

// WIFI
const char* ssid = "selesai";
const char* password = "22222222";

// TELEGRAM
String BOTtoken = "8733038130:AAE-INunOp97XEmmbhg661vw7vQfqmNgmV8";
String CHAT_ID  = "8518442862";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// GPS
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// ESP32-CAM
HardwareSerial camSerial(1);

// PIN
#define GPS_RX 16
#define GPS_TX 17

#define CAM_RX 2
#define CAM_TX 4

unsigned long lastTimeBotRan;
int botRequestDelay = 1500;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // CAM
  camSerial.begin(115200, SERIAL_8N1, CAM_RX, CAM_TX);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setInsecure();

  Serial.println("System Ready");
}

// ================= LOOP =================
void loop() {

  // Baca GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // Telegram
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}

// ================= HANDLE COMMAND =================
void handleMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {

    String text = bot.messages[i].text;

    if (text == "/start") {
      bot.sendMessage(CHAT_ID,
        "🤖 Smart Farming System\n\n"
        "/photo - Ambil foto\n"
        "/location - Lokasi sapi\n",
        "");
    }

    else if (text == "/location") {
      sendLocation();
    }

    else if (text == "/photo") {
      camSerial.println("CAPTURE");
      bot.sendMessage(CHAT_ID, "📸 Ambil foto...", "");
    }
  }
}

// ================= KIRIM LOKASI =================
void sendLocation() {
  if (!gps.location.isValid()) {
    bot.sendMessage(CHAT_ID, "❌ GPS belum fix", "");
    return;
  }

  String lat = String(gps.location.lat(), 6);
  String lng = String(gps.location.lng(), 6);

  String maps = "https://maps.google.com/?q=" + lat + "," + lng;

  bot.sendMessage(CHAT_ID, "📍 Lokasi:\n" + maps, "");
}