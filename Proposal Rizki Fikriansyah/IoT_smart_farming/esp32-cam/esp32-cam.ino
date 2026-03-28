#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// WIFI
const char* ssid = "Kiki S.Kom";
const char* password = "22222222";

// TELEGRAM
String BOTtoken = "8733038130:AAE-INunOp97XEmmbhg661vw7vQfqmNgmV8";
String CHAT_ID  = "8518442862";

// CAMERA PIN (AI THINKER)
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

WiFiClientSecure client;

// ================= CAMERA =================
void startCamera() {
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

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
}

// ================= SEND PHOTO =================
void sendPhoto() {
  camera_fb_t * fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.println("Connecting to Telegram...");

  client.setInsecure();

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Connection failed");
    return;
  }

  String head = "--ESP32\r\nContent-Disposition: form-data; name=\"chat_id\";\r\n\r\n" + CHAT_ID + 
                "\r\n--ESP32\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--ESP32--\r\n";

  uint32_t imageLen = fb->len;
  uint32_t totalLen = head.length() + imageLen + tail.length();

  client.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Length: " + String(totalLen));
  client.println("Content-Type: multipart/form-data; boundary=ESP32");
  client.println();

  client.print(head);

  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;

  for (size_t n = 0; n < fbLen; n += 1024) {
    if (n + 1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    } else {
      size_t remainder = fbLen - n;
      client.write(fbBuf, remainder);
    }
  }

  client.print(tail);

  esp_camera_fb_return(fb);

  Serial.println("Photo sent!");
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

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  startCamera();

  Serial.println("Ketik 'FOTO' di Serial Monitor");
}

// ================= LOOP =================
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');

    cmd.trim();

    if (cmd == "FOTO") {
      Serial.println("Ambil foto...");
      sendPhoto();
    }
  }
}