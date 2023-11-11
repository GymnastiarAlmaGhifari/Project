#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
// #include <HardwareSerial.h>

const char* ssid = "Barokah";
const char* password = "gakdipassword";
// const char* ssid = "SMK TRUNOJOYO";
// const char* password = "tanyamasoki";

// const char* server = "192.168.2.12";
const char* server = "192.168.192.72";

String serverPath = "/api/socket/image";

const int serverPort = 3000;

// HardwareSerial esp8266Serial(2); 
String id = "";

WiFiClient client;

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

// #define LED_Flash 4

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  // esp8266Serial.begin(115200);  // Inisialisasi komunikasi serial ke ESP8266
  WiFi.mode(WIFI_STA);
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print(".");
    delay(500);
  }
  // Serial.println();
  // Serial.print("ESP32-CAM IP Address: ");
  // Serial.println(WiFi.localIP());

//  pinMode(LED_Flash, OUTPUT);

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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}



// void loop() {
//   if (Serial.available()) {
//     String id = Serial.readStringUntil('#');
//     Serial.println("Raw ID: " + id); // Cetak ID sebelum membersihkan
//     Serial.println("Cleaned ID: " + id); // Cetak ID yang sudah dibersihkan
//     if (!id.isEmpty()) { // Pastikan ID tidak kosong sebelum mengirim foto
//       sendPhoto(id);
//     }
//   }
// }


// String cleanFilename(String filename) {
//   // Hapus karakter yang tidak valid dalam nama file
//   String cleanName = "";
//   for (int i = 0; i < filename.length(); i++) {
//     char c = filename.charAt(i);
//     if (isAlphaNumeric(c) || c == '.' || c == '-' || c == '_') {
//       cleanName += c;
//     }
//   }
//   return cleanName;
// }

void loop() {
  if (Serial.available()) {
    String id = Serial.readStringUntil('#');
    // Serial.println("Received ID: " + id);
    if (!id.isEmpty()) {
      // String cleanedId = cleanFilename(id);
      sendPhoto(id);
    }
  }
}

String sendPhoto(String id) {
    // String cleanId = cleanFilename(id); // Membersihkan ID
  String getAll;
  String getBody;
    String encodedId = urlencode(id); // Encode the ID


  //  digitalWrite(LED_Flash,HIGH);

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    // Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  if (client.connect(server, serverPort)) {
    // Serial.println("Connection successful!");
    // String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
// String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"" + id + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"" + encodedId + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
        
    // String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"" + id + ".jpeg\"\r\nContent-Type: image/jpeg\r\n\r\n";
      // String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"" + cleanId + ".jpeg\"\r\nContent-Type: image/jpeg\r\n\r\n";


    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

  client.println("POST " + serverPath + "?id=" + encodedId + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    client.println();
    client.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n += 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }
    client.print(tail);

    esp_camera_fb_return(fb);

    int timeoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + timeoutTimer) > millis()) {
      // Serial.print(".");
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length() == 0) {
            state = true;
          }
          getAll = "";
        } else if (c != '\r') {
          getAll += String(c);
        }
        if (state == true) {
          getBody += String(c);
        }
        startTimer = millis();
      }
      if (getBody.length() > 0) {
        break;
      }
    }
    // Serial.println();
    client.stop();
    // Serial.println(getBody);
  } else {
    getBody = "Connection to " + String(server) + " failed.";
    // Serial.println(getBody);
  }
  return getBody;
}

String urlencode(String str) {
  const char *msg = str.c_str();
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";
  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}
