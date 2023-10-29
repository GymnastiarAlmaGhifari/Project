#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

const char* ssid = "SMK TRUNOJOYO";
const char* password = "tanyamasoki";
const uint8_t RST_PIN = 0; // Sesuaikan dengan nomor GPIO yang sesuai
const uint8_t SS_PIN = 2; // Sesuaikan dengan nomor GPIO yang sesuai

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  connectWiFi();
  Serial.println("Connected to WiFi");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("RFID Tag:");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    mfrc522.PICC_HaltA();
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
