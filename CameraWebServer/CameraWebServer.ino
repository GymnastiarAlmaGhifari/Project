#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h" // If base64 functions are provided by a separate library
#include "esp_camera.h"
const char* ssid = "SMK TRUNOJOYO";
const char* password = "tanyamasoki";
//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define LED_Flash 4
String dataIn;
String imageFile = "";

HTTPClient http; // Mendeklarasikan objek HTTPClient di luar setup() dan loop()


String id = "";
const String url = "http://192.168.43.18:3000/api/socket/image";

//const String url = "http://absensi.xyz/api/absensifotojson";
//const String url = "http://192.168.1.5/absensi/api/absensifotojson";
void setup()
{
 WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
 
 Serial.begin(9600);
 delay(10);
 WiFi.begin(ssid, password); 
 while (WiFi.status() != WL_CONNECTED) { //Check for the connection
 delay(500);
 Serial.println("Connecting..");
 }
 Serial.print("Connected to the WiFi network with IP: ");
 Serial.println(WiFi.localIP());
 pinMode(LED_Flash, OUTPUT);
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
 
 //init with high specs to pre-allocate larger buffers
 if(psramFound()){
 config.frame_size = FRAMESIZE_UXGA;
 config.jpeg_quality = 10; //0-63 lower number means higher quality
 config.fb_count = 2;
 } else {
 config.frame_size = FRAMESIZE_SVGA;
 config.jpeg_quality = 12; //0-63 lower number means higher quality
 config.fb_count = 1;
 }
 
 // camera init
 esp_err_t err = esp_camera_init(&config);
 if (err != ESP_OK) {
 Serial.printf("Camera init failed with error 0x%x", err);
 delay(1000);
 ESP.restart();
 }
 //drop down frame size for higher initial frame rate
 sensor_t * s = esp_camera_sensor_get();
 s->set_framesize(s, FRAMESIZE_VGA); // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

// void loop() {
//   if (Serial.available()) {
//     dataIn = Serial.readStringUntil('#'); // kirim data akhiran harus ditambah #
//     if (dataIn.length() >= 9) {
//       if (WiFi.status() == WL_CONNECTED) {
//         rfid = dataIn;
//         digitalWrite(LED_Flash, HIGH);
//         delay(200);
//         Photo2Base64();
//         digitalWrite(LED_Flash, LOW);

//         HTTPClient http;
//         String apiUrl = url + "?id=" + rfid;
//         http.begin(apiUrl); // Menggunakan URL yang telah diubah

//         // Tidak perlu lagi mengirim 'key', 'iddev', dan 'rfid' sebagai parameter
//         // Anda hanya perlu mengirim 'foto' yang sudah Anda peroleh dari imageFile

//         http.addHeader("Content-Type", "application/x-www-form-urlencoded");

//         int httpResponseCode = http.POST("foto=" + imageFile);
//         Serial.println(rfid);

//         if (httpResponseCode > 0) {
//           Serial.println(httpResponseCode);
//           Serial.println(http.getString());
//         } else {
//           Serial.print("Error on sending request: ");
//           Serial.println(httpResponseCode);
//         }
//         http.end();
//         dataIn = "";
//       } else {
//         Serial.println("Error in WiFi connection");
//       }
//     } else {
//       Serial.println("Data RFID salah");
//     }
//   }
// }
void loop() {
  String id = "popo"; // Mengatur nilai id secara langsung
  digitalWrite(LED_Flash, HIGH);
  delay(200);
  Photo2Base64();
  digitalWrite(LED_Flash, LOW);

  String apiUrl = url + "?id=" + id;
  http.begin(apiUrl);

  // Set header untuk FormData
  String formDataBoundary = "--------------------------boundary";
  String formDataHeader = "--" + formDataBoundary + "\r\nContent-Disposition: form-data; name=\"foto\"; filename=\"" + id + ".jpeg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String formDataFooter = "\r\n--" + formDataBoundary + "--\r\n";

  // Mengirim data FormData
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + formDataBoundary);
  http.addHeader("Content-Length", String(formDataHeader.length() + imageFile.length() + formDataFooter.length()));

  int httpResponseCode = http.POST(formDataHeader + imageFile + formDataFooter);

  Serial.println(id);

  if (httpResponseCode > 0) {
    Serial.println(httpResponseCode);
    Serial.println(http.getString());
  } else {
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(5000); // Tambahkan penundaan agar tidak mengirim terus-menerus
}


String Photo2Base64() {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) {
      Serial.println("Camera capture failed");
      return "";
    }
  
    String imageFile = "data:image/jpeg;base64,";
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }

    esp_camera_fb_return(fb);
    
    return imageFile;
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