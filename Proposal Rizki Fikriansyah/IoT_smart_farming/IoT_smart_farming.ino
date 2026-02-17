#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>

const char* ssid = "rizki";
const char* password = "25022025";

String BOTtoken = "ISI_BOT_TOKEN";
String CHAT_ID  = "ISI_CHAT_ID";

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

TinyGPSPlus gps;
#define gpsSerial Serial2

#define FLASH_LED_PIN 4
bool flashState = LOW;

bool sendPhoto = false;

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// ================= CAMERA PIN CONFIG (AI THINKER) =================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// ================================================================

void configInitCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size   = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count     = 2;
  } else {
    config.frame_size   = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count     = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    ESP.restart();
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    
    if (text == "/photo") {
      sendPhoto = true;
    }

    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
    }
  }
}

String sendPhotoTelegram() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "Capture failed";
  }

  if (clientTCP.connect("api.telegram.org", 443)) {
    String head = "--boundary\r\nContent-Disposition: form-data; name=\"chat_id\";\r\n\r\n" + CHAT_ID +
                  "\r\n--boundary\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";

    String tail = "\r\n--boundary--\r\n";

    clientTCP.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: api.telegram.org");
    clientTCP.println("Content-Type: multipart/form-data; boundary=boundary");
    clientTCP.println("Content-Length: " + String(fb->len + head.length() + tail.length()));
    clientTCP.println();
    clientTCP.print(head);
    clientTCP.write(fb->buf, fb->len);
    clientTCP.print(tail);

    esp_camera_fb_return(fb);
  }

  clientTCP.stop();
  return "Photo sent";
}

void sendGPSLocation() {
  if (gps.location.isValid()) {
    String message = "Lokasi Ternak:\n";
    message += "Lat: " + String(gps.location.lat(), 6) + "\n";
    message += "Lng: " + String(gps.location.lng(), 6);

    bot.sendMessage(CHAT_ID, message, "");
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // GPS RX, TX

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  configInitCamera();

  WiFi.begin(ssid, password);
  clientTCP.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
}

void loop() {

  // ===== READ GPS =====
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // ===== HANDLE PHOTO REQUEST =====
  if (sendPhoto) {
    sendPhotoTelegram();
    sendPhoto = false;
  }

  // ===== TELEGRAM BOT =====
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }

  // ===== PERIODIC GPS SEND (SMART FARMING FUNCTION) =====
  static unsigned long lastGPS = 0;
  if (millis() - lastGPS > 10000) {   // kirim tiap 10 detik
    sendGPSLocation();
    lastGPS = millis();
  }
}