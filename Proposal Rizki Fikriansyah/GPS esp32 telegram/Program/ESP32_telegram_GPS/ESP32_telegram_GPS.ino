#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Rejoagung";
const char* password = "rizkaaja";

// inisialisasi Bot Token
#define BOTtoken "5731758396:AAHS2uqNV-vfdydmaQllZ-G7XQSHw7n0kfs"  // Bot Token dari BotFather

// chat id dari @myidbot
#define CHAT_ID "5795193300"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 100;
unsigned long lastTimeBotRan;

#include <TinyGPS++.h>
#include <HardwareSerial.h>

HardwareSerial GPSSerial(1);
TinyGPSPlus gps;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    while(GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }
   if (gps.charsProcessed() > 10) {
    float currentLat = gps.location.lat();
    float currentLng = gps.location.lng();

    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring Lokasi GPS\n\n";
      control += "/Lokasi Untuk Mengetahui lokasi saat ini \n";
      bot.sendMessage(chat_id, control, "");
    }

      
  if (text == "/Lokasi"){
     String lokasi = "Lokasi : https://www.google.com/maps/@";
      lokasi +=String(currentLat,6);
      lokasi +=",";
      lokasi +=String(currentLng,6);
      lokasi +=",21z?entry=ttu";
      bot.sendMessage(chat_id, lokasi, "");
  }  
 
   }
 
  }
}

void setup() {
  Serial.begin(115200);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17);
  // Koneksi Ke Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}