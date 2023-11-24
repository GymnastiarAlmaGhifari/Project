#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

const char* ssid = "Barokah";
const char* password = "gakdipassword";
// const char* ssid = "SMK TRUNOJOYO";
// const char* password = "tanyamasoki";
// const char* server = "192.168.2.12";
const char* server = "192.168.7.72";
const int port = 3000;

constexpr uint8_t RST_PIN = 0;  // Gunakan nomor GPIO yang sesuai
constexpr uint8_t SS_PIN = 2;   // Gunakan nomor GPIO yang sesuai
constexpr uint8_t Buzzer = 15;  // Gunakan nomor GPIO yang sesuai


MFRC522 rfid(SS_PIN, RST_PIN); // Instance dari kelas MFRC522
MFRC522::MIFARE_Key key;
SoftwareSerial esp32Serial(-1, 16); // Hanya menggunakan pin GPIO16 (D0) sebagai TX



WiFiClient NodeMCU;
String response; // Deklarasi 'response' sebagai variabel String

void setup() {
  Serial.begin(9600);
  esp32Serial.begin(115200);
  SPI.begin(); // Init bus SPI
  rfid.PCD_Init(); // Init MFRC522
  pinMode(Buzzer, OUTPUT); // Tentukan pin Buzzer sebagai OUTPUT


  // Terhubung dengan Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
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
      String id = responseJson["rfidData"][0]["id_kambing"].as<String>();
      // String tanggal_lahir = responseJson["rfidData"][0]["tanggal_lahir"].as<String>();

      // Langkah 3: Membangun payload JSON untuk notifikasi
      StaticJsonDocument<200> notificationData;
      notificationData["message_notifications"] = "RFID Tag: " + tag + ", Nama Kambing: " + namaKambing;
      notificationData["tag_id"] = id;

      String dataToSend = String(id) + "#";
      for (size_t i = 0; i < dataToSend.length(); i++) {
          esp32Serial.print(dataToSend.charAt(i));
          delay(20); // Tambahkan penundaan kecil di sini
      }


      Serial.println("id: " + dataToSend);

      String notificationPayload;
      serializeJson(notificationData, notificationPayload);

      // Langkah 4: Kirim permintaan POST ke API notifikasi dengan payload notifikasi
      HTTPClient notificationClient;

      // Membangun URL untuk notifikasi
      String notificationUrl = "http://" + String(server) + ":" + String(port) + "/api/socket/notif";
      notificationClient.begin(NodeMCU, notificationUrl);
      notificationClient.addHeader("Content-Type", "application/json");
      int notificationResponseCode = notificationClient.POST(notificationPayload);
      tone(Buzzer, 1000, 500); // Frekuensi 2000 Hz selama 500 ms


      if (notificationResponseCode != 200) {
        Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
      }

    } else if (httpResponseCode == 404) {
      // Status 404: RFID tidak ditemukan dalam database, kirimkan notifikasi dengan pesan yang sesuai
      DynamicJsonDocument responseJson(200);
      deserializeJson(responseJson, response);

      String errorMessage = "RFID:"+ tag + "tidak ditemukan dalam database";

      StaticJsonDocument<200> notificationData;
      notificationData["message_notifications"] = errorMessage;

      String notificationPayload;
      serializeJson(notificationData, notificationPayload);
      // Serial.println("Response: " + response);


      // Kirim notifikasi dengan pesan yang sesuai
      HTTPClient notificationClient;

      // Membangun URL untuk notifikasi
      String notificationUrl = "http://" + String(server) + ":" + String(port) + "/api/socket/notif";
      notificationClient.begin(NodeMCU, notificationUrl);
      notificationClient.addHeader("Content-Type", "application/json");
      int notificationResponseCode = notificationClient.POST(notificationPayload);

        for (int i = 0; i < 3; i++) {
          tone(Buzzer, 1000, 250); // Nada pertama, frekuensi 1000 Hz selama 250 ms
          delay(500); // Tunggu sebentar sebelum nada berikutnya
        }


      if (notificationResponseCode != 200) {
        // Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
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
        // Serial.println("Failed to send notification. Status code: " + String(notificationResponseCode));
      }
    }

    // Reset modul RFID dan tambahkan penundaan jika diperlukan
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    http.end();
  } 
  

  delay(1000); // Penundaan selama 1 detik antara pembacaan
}
