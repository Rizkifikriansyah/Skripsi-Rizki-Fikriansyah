#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

// ================= WIFI =================
const char* ssid = "selesai";
const char* password = "22222222";

// ================= TELEGRAM =================
String BOTtoken = "8733038130:AAE-INunOp97XEmmbhg661vw7vQfqmNgmV8";
String CHAT_ID  = "8518442862";

// ================= CAMERA PIN =================
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

// ================= INIT CAMERA =================
void initCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

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
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000; // 🔥 stabil
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Kamera gagal");
    ESP.restart();
  }

  // warmup
  for (int i = 0; i < 5; i++) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
    delay(200);
  }

  Serial.println("✅ Kamera siap");
}

// ================= KIRIM PESAN =================
void sendMessage(String text) {
  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("🔌 Connect Telegram (message)...");

  if (!client.connect("149.154.167.220", 443)) {
    Serial.println("❌ Gagal kirim pesan");
    return;
  }

  String url = "/bot" + BOTtoken + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + text;

  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Connection: close");
  client.println();

  delay(1000);
  Serial.println("📨 Pesan terkirim");
}

// ================= KIRIM FOTO =================
void sendPhoto() {

  camera_fb_t * fb = NULL;

  // retry ambil foto
  for (int i = 0; i < 5; i++) {
    fb = esp_camera_fb_get();

    if (fb && fb->len > 5000) {
      Serial.println("✅ Foto valid");
      break;
    }

    Serial.println("⚠️ Retry ambil foto...");
    if (fb) esp_camera_fb_return(fb);
    delay(500);
  }

  if (!fb) {
    Serial.println("❌ Gagal capture");
    sendMessage("❌ Kamera gagal capture");
    return;
  }

  Serial.print("📸 Ukuran foto: ");
  Serial.println(fb->len);

  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("🔌 Connect Telegram (photo)...");

  if (!client.connect("149.154.167.220", 443)) {
    Serial.println("❌ Gagal koneksi Telegram");
    sendMessage("❌ Gagal koneksi Telegram");
    esp_camera_fb_return(fb);
    return;
  }

  String boundary = "ESP32CAM";

  String head =
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + CHAT_ID + "\r\n"
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"photo\"; filename=\"esp32.jpg\"\r\n"
    "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t totalLen = fb->len + head.length() + tail.length();

  client.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(totalLen));
  client.println("Connection: close");
  client.println();

  client.print(head);

  for (size_t i = 0; i < fb->len; i += 1024) {
    size_t chunk = (i + 1024 < fb->len) ? 1024 : fb->len - i;
    client.write(fb->buf + i, chunk);
  }

  client.print(tail);

  esp_camera_fb_return(fb);

  // ===== RESPONSE =====
  String response = "";
  long timeout = millis();

  while (client.connected() && millis() - timeout < 10000) {
    while (client.available()) {
      char c = client.read();
      response += c;
    }
  }

  Serial.println("==== RESPONSE ====");
  Serial.println(response);
  Serial.println("==================");

  if (response.indexOf("\"ok\":true") != -1) {
    Serial.println("✅ FOTO BERHASIL TERKIRIM 🔥");
    sendMessage("✅ Foto berhasil dikirim 📸");
  } else {
    Serial.println("❌ GAGAL KIRIM FOTO");
    sendMessage("❌ Gagal kirim foto");
  }

  client.stop();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");

  initCamera();

  sendMessage("🤖 ESP32-CAM ONLINE");
}

// ================= LOOP =================
void loop() {
  sendPhoto();
  delay(20000);
}