#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> // version 6.13
#include <SPI.h>
#include <RFID.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
SoftwareSerial camera(-1, D0); // RX, TX (RX tidak dipakai) 
D0 (nodemcu) -> UOR(ESP32 CAM)
LiquidCrystal_I2C lcd(0x27, 20, 4); // sesuaikan alamat 
i2c (0x27) dengan alamat i2c kalian
#define SS_PIN D4
#define RST_PIN D3
#define Buzzer D8
const char* wifiName = "kere kere hore";
const char* wifiPass = "lakasandine";
const String iddev = "22";
String hostMode = 
"http://192.168.43.18/tugasakhir/api/getmodejson?key=cdd345280
ccc8c783f0545b3be200a08&iddev=" + iddev;
String hostSCAN = 
"http://192.168.43.18/tugasakhir/api/absensijson?key=cdd345280
ccc8c783f0545b3be200a08&iddev=" + iddev;
String hostADD = 
"http://192.168.43.18/tugasakhir/api/addcardjson?key=cdd345280
ccc8c783f0545b3be200a08&iddev=" + iddev;
//String hostMode = 
"http://192.168.43.106/absensi/api/getmodejson?key=asdkjWEQEDa
sd12ksnd&iddev=" + iddev;
//String hostSCAN = 
"http://192.168.43.106/absensi/api/absensijson?key=asdkjWEQEDa
sd12ksnd&iddev=" + iddev;
//String hostADD = 
"http://192.168.43.106/absensi/api/addcardjson?key=asdkjWEQEDa
sd12ksnd&iddev=" + iddev;
String ModeAlat = "";
RFID rfid(SS_PIN, RST_PIN);
void setup() {
Serial.begin(115200);
 camera.begin(9600);
 SPI.begin();
 rfid.init();
 delay(10);
 pinMode(Buzzer, OUTPUT);
 
 Wire.begin(D2,D1);
 lcd.begin();
 lcd.home ();
 lcd.print("RFID Reader Absensi"); 
 delay (1000);
 Serial.println();
 
 Serial.print("Connecting to ");
 Serial.println(wifiName);
 lcd.setCursor(0,1);
 lcd.print("Connecting to");
 lcd.setCursor(0,2);
 lcd.print(wifiName); 
 WiFi.begin(wifiName, wifiPass);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP()); //You can get IP address 
assigned to ESP
 ModeDevice();
}
void ModeDevice(){
 HTTPClient http;
 Serial.print("Request Link:");
 Serial.println(hostMode);
 
 http.begin(hostMode);
 
 int httpCode = http.GET(); //Send the request
 String payload = http.getString(); //Get the response 
payload from server
 Serial.print("Response Code:"); //200 is OK
 Serial.println(httpCode); //Print HTTP return code
 Serial.print("Returned data from Server:");
 Serial.println(payload); //Print request response payload
 if(httpCode == 200)
 {
 DynamicJsonDocument doc(1024);
 
 // Parse JSON object
 auto error = deserializeJson(doc, payload);
 if (error) {
 Serial.print(F("deserializeJson() failed with code "));
 Serial.println(error.c_str());
 return;
 }
 
 // Decode JSON/Extract values
 String responStatus = doc["status"].as<String>();
 String responMode = doc["mode"].as<String>();
 String responKet = doc["ket"].as<String>();
 Serial.println();
 Serial.print("status : ");
 Serial.println(responStatus);
 
 Serial.print("mode : ");
 Serial.println(responMode);
 
 Serial.print("ket : ");
 Serial.println(responKet);
 Serial.println("-------------------");
 Serial.println();
 lcd.clear();
 lcd.print("System Absensi RFID");
 if (responMode == "SCAN"){
 ModeAlat = "SCAN";
 lcd.setCursor(0,1);
 lcd.print(" SCAN Your RFID Card");
 }else if (responMode == "ADD"){
 ModeAlat = "ADD";
 lcd.setCursor(0,1);
 lcd.print(" ADD Your RFID Card");
 }else{
 ModeAlat = "";
 lcd.setCursor(0,2);
 lcd.print(responKet);
 }
 }
 else
 {
 Serial.println("Error in response");
 }
 http.end();
 delay(100);
}

void loop() {
 if (ModeAlat == "SCAN"){
 Serial.println("SCAN RFID CARD");
 if (rfid.isCard()) {
 if (rfid.readCardSerial()) {
 //Serial.println(rfid.serNum.length());
 BEEP(2, 200, 200);
 Serial.println("");
 Serial.println("Card found");
 String RFID = String(rfid.serNum[0],HEX) +"-"+ 
String(rfid.serNum[1],HEX) +"-"+ String(rfid.serNum[2],HEX) 
+"-"+ String(rfid.serNum[3],HEX) +"-"+ 
String(rfid.serNum[4],HEX);
 lcd.setCursor(0,2);
 lcd.print("UID:");
 lcd.print(RFID);
 lcd.print(" ");
 Serial.println(RFID);
 Serial.println("");
 String host = hostSCAN;
 host += "&rfid=";
 host += RFID;
 HTTPClient http;
 Serial.print("Request Link:");
 Serial.println(host);
 
 http.begin(host);
 
 int httpCode = http.GET(); //Send the GET 
request
 String payload = http.getString(); //Get the 
response payload from server
 
 Serial.print("Response Code:"); //200 is OK
 Serial.println(httpCode); //Print HTTP return 
code
 
 Serial.print("Returned data from Server:");
 Serial.println(payload); //Print request response 
payload
 
 if(httpCode == 200)
 {
 DynamicJsonDocument doc(1024);
 
 // Parse JSON object
 auto error = deserializeJson(doc, payload);
 if (error) {
 Serial.print(F("deserializeJson() failed with 
code "));
 Serial.println(error.c_str());
return;
 }
 
 // Decode JSON/Extract values
 String responStatus = doc["status"].as<String>();
 String responKet = doc["ket"].as<String>();
 if (responStatus == "success"){
 camera.print(RFID);
 camera.print("#");
 Serial.println("Kirim UID RFID ke ESP32 CAM");
 delay(1000);
 }
 lcd.setCursor(0,3);
 lcd.print(responKet);
 
 Serial.println();
 Serial.print("status : ");
 Serial.println(responStatus);
 
 Serial.print("ket : ");
 Serial.println(responKet);
 Serial.println("-------------------");
 Serial.println();
 delay(1000);
 lcd.setCursor(0,3);
 lcd.print(" ");
 }
 }
 }else{
 Serial.println("WAITING RFID CARD");
 lcd.setCursor(0,2);
 lcd.print("Menunggu Kartu RFID ");
 }
 rfid.halt();
 delay(1000);
 }else if (ModeAlat == "ADD"){
 Serial.println("ADD RFID CARD");
 if (rfid.isCard()) {
 if (rfid.readCardSerial()) {
 //Serial.println(rfid.serNum.length());
 BEEP(2, 200, 200);
 
 Serial.println("");
 Serial.println("Card found");
 String RFID = String(rfid.serNum[0],HEX) +"-"+ 
String(rfid.serNum[1],HEX) +"-"+ String(rfid.serNum[2],HEX) 
+"-"+ String(rfid.serNum[3],HEX) +"-"+ 
String(rfid.serNum[4],HEX);
 lcd.setCursor(0,2);
 lcd.print("UID:");
 lcd.print(RFID);
 lcd.print(" ");
Serial.println(RFID);
 Serial.println("");
 String host = hostADD;
 host += "&rfid=";
 host += RFID;
 HTTPClient http;
 Serial.print("Request Link:");
 Serial.println(host);
 
 http.begin(host);
 
 int httpCode = http.GET(); //Send the GET 
request
 String payload = http.getString(); //Get the 
response payload from server
 
 Serial.print("Response Code:"); //200 is OK
 Serial.println(httpCode); //Print HTTP return 
code
 
 Serial.print("Returned data from Server:");
 Serial.println(payload); //Print request response 
payload
 
 if(httpCode == 200)
 {
 DynamicJsonDocument doc(1024);
 
 // Parse JSON object
 auto error = deserializeJson(doc, payload);
 if (error) {
 Serial.print(F("deserializeJson() failed with 
code "));
 Serial.println(error.c_str());
 return;
 }
 
 // Decode JSON/Extract values
 String responStatus = doc["status"].as<String>();
 String responKet = doc["ket"].as<String>();
 lcd.setCursor(0,3);
 lcd.print(responKet);
 
 Serial.println();
 Serial.print("status : ");
 Serial.println(responStatus);
 
 Serial.print("ket : ");
 Serial.println(responKet);
 Serial.println("-------------------");
 Serial.println();

delay(1000);
 lcd.setCursor(0,3);
 lcd.print(" ");
 }
 }
 }else{
 Serial.println("WAITING RFID CARD");
 lcd.setCursor(0,2);
 lcd.print("Menunggu Kartu RFID");
 }
 rfid.halt();
 delay(1000);
 }else{
 Serial.println("Tidak Mendapatkan MODE ALAT dari server");
 Serial.println("Cek IP Server dan URL");
 Serial.println("Restart NodeMCU");
 lcd.setCursor(0,1);
 lcd.print("MODE ALAT ERROR");
 lcd.setCursor(0,2); 
 Serial.println("Cek IP Server dan URL");
 lcd.setCursor(0,3);
 Serial.println("Restart NodeMCU");
 delay(10000); 
 ModeDevice();
 }
}
// function sound beep = BEEP(how many, delay on in ms, delay 
off in ms);
void BEEP(byte c, int wait1, int wait2){
 for(byte b=0; b<c; b++){
 digitalWrite(Buzzer,HIGH);
 delay(wait1);
 digitalWrite(Buzzer,LOW);
 delay(wait2);
 }
}
