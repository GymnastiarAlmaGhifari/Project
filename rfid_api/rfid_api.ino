#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "SMK TRUNOJOYO";
const char* password = "tanyamasoki";
const char* server = "192.168.2.6";
const int port = 3000;

constexpr uint8_t RST_PIN = 0;  // Gunakan nomor GPIO yang sesuai
constexpr uint8_t SS_PIN = 2;   // Gunakan nomor GPIO yang sesuai

MFRC522 rfid(SS_PIN, RST_PIN); // Instance dari kelas MFRC522
MFRC522::MIFARE_Key key;

WiFiClient NodeMCU;

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init bus SPI
  rfid.PCD_Init(); // Init MFRC522

  // Terhubung dengan Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  String response; // Deklarasi 'response' sebagai variabel String

  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    String tag = "";
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println("RFID Tag: " + tag);

    // Langkah 1: Kirim permintaan GET ke API untuk mengambil data RFID
    HTTPClient http;

    // Membangun URL
    String url = "http://" + String(server) + ":" + String(port) + "/api/rfid?rfid=" + tag;

    http.begin(NodeMCU, url);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      response = http.getString();

      // Langkah 2: Mengurai respons untuk mengekstrak nilai "nama_kambing"
      DynamicJsonDocument responseJson(200);
      deserializeJson(responseJson, response);

      // Ekstrak "nama_kambing" dari respons
      String namaKambing = responseJson["rfidData"][0]["nama_kambing"].as<String>();

      // Langkah 3: Membangun payload JSON untuk notifikasi
      StaticJsonDocument<200> notificationData;
      notificationData["message_notifications"] = "RFID Tag: " + tag + ", Nama Kambing: " + namaKambing;

      String notificationPayload;
      serializeJson(notificationData, notificationPayload);

      // Langkah 4: Kirim permintaan POST ke API notifikasi dengan payload notifikasi
      HTTPClient notificationClient;

      // Membangun URL untuk notifikasi
      String notificationUrl = "http://" + String(server) + ":" + String(port) + "/api/socket/notif";
      notificationClient.begin(NodeMCU, notificationUrl);
      notificationClient.addHeader("Content-Type", "application/json");
      int notificationResponseCode = notificationClient.POST(notificationPayload);

      if (notificationResponseCode != 200) {
        Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
      }

    } else if (httpResponseCode == 404) {
      // Status 404: RFID tidak ditemukan dalam database, kirimkan notifikasi dengan pesan yang sesuai
      DynamicJsonDocument responseJson(200);
      deserializeJson(responseJson, response);

      String errorMessage = "RFID tidak ditemukan dalam database";

      StaticJsonDocument<200> notificationData;
      notificationData["message_notifications"] = errorMessage;

      String notificationPayload;
      serializeJson(notificationData, notificationPayload);
      Serial.println("Response: " + response);


      // Kirim notifikasi dengan pesan yang sesuai
      HTTPClient notificationClient;

      // Membangun URL untuk notifikasi
      String notificationUrl = "http://" + String(server) + ":" + String(port) + "/api/socket/notif";
      notificationClient.begin(NodeMCU, notificationUrl);
      notificationClient.addHeader("Content-Type", "application/json");
      int notificationResponseCode = notificationClient.POST(notificationPayload);

      if (notificationResponseCode != 200) {
        Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
      }
    } else {
      // Kesalahan umum lainnya, kirim notifikasi dengan pesan kesalahan umum
      StaticJsonDocument<200> notificationData;
      notificationData["message_notifications"] = "Terjadi kesalahan dalam memproses permintaan.";

      String notificationPayload;
      serializeJson(notificationData, notificationPayload);

      // Kirim notifikasi dengan pesan kesalahan umum
      HTTPClient notificationClient;

      // Membangun URL untuk notifikasi
      String notificationUrl = "http://" + String(server) + ":" + String(port) + "/api/socket/notif";
      notificationClient.begin(NodeMCU, notificationUrl);
      notificationClient.addHeader("Content-Type", "application/json");
      int notificationResponseCode = notificationClient.POST(notificationPayload);

      if (notificationResponseCode != 200) {
        Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
      }
    }

    // Reset modul RFID dan tambahkan penundaan jika diperlukan
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    http.end();
  } 
  

  delay(1000); // Penundaan selama 1 detik antara pembacaan
}
