#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD dengan alamat I2C, 20 kolom, dan 4 baris
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Definisi Pin
#define DHTTYPE DHT11
#define dht_pin 33
#define PIR 12
#define PinSoil 34
#define SensorHujan 15
#define trig 26
#define echo 27 
#define RelayPompa 5
#define RelayPompa2 18
#define servo_pin 2

// Threshold untuk Logika Sistem
const int soil_dry_threshold_percent = 30; // Siram jika kelembapan di bawah 30%
const int reservoir_kosong_cm = 25;        // Jarak > 25cm dianggap kosong
const int reservoir_penuh_cm = 10;         // Jarak < 10cm dianggap penuh

// Kredensial Wi-Fi & MQTT
const char* ssid = "Zuppa";
const char* password = "Ujang123";
const char* mqtt_server = "broker.hivemq.com";

// Topik MQTT
#define IN_TOPIC_PUMP1 "IN_TOMBOL_POMPA1"
#define IN_TOPIC_PUMP2 "IN_TOMBOL_POMPA2"

// Inisialisasi Objek
DHT dht(dht_pin, DHTTYPE);
Servo servo1;
WiFiClient espClient;
PubSubClient client(espClient);

// Variabel Global
unsigned long lastMsg = 0;
String StatusR1 = "N/A";
String StatusP1 = "Off";
String StatusPs1 = "N/A";
String StatusP2 = "Off";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menyambungkan ke ");
  Serial.println(ssid);
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  String topicStr = topic;

  if (topicStr == IN_TOPIC_PUMP1) {
    if (message == "ON") {
      digitalWrite(RelayPompa, LOW); // LOW menyalakan relay
      client.publish("OUT_ARRASYID_POMPA", "On");
      Serial.println("Pompa 1 Manual : On");
    } else if (message == "OFF") {
      digitalWrite(RelayPompa, HIGH); // HIGH mematikan relay
      client.publish("OUT_ARRASYID_POMPA", "Off");
      Serial.println("Pompa 1 Manual : Off");
    }
  } else if (topicStr == IN_TOPIC_PUMP2) {
    if (message == "ON") {
      digitalWrite(RelayPompa2, LOW);
      client.publish("OUT_ARRASYID_KERAN", "On");
      Serial.println("Pompa 2 Manual : On");
    } else if (message == "OFF") {
      digitalWrite(RelayPompa2, HIGH);
      client.publish("OUT_ARRASYID_KERAN", "Off");
      Serial.println("Pompa 2 Manual : Off");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("terhubung");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQTT Connected!");
      delay(2000);
      client.subscribe(IN_TOPIC_PUMP1);
      client.subscribe(IN_TOPIC_PUMP2);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void ServoAktif() {
  for (int pos = 0; pos <= 180; pos += 30) {
    servo1.write(pos);
    delay(15);
  }
  for (int pos = 180; pos >= 0; pos -= 30) {
    servo1.write(pos);
    delay(15);
  }
  servo1.write(0); // Kembali ke posisi awal
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(PIR, INPUT);
  pinMode(PinSoil, INPUT);
  pinMode(SensorHujan, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(RelayPompa, OUTPUT);
  pinMode(RelayPompa2, OUTPUT);
  
  servo1.attach(servo_pin);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 1);
  lcd.print("  Smart Irrigation  ");
  lcd.setCursor(6, 2);
  lcd.print("System");
  delay(2000);
  lcd.clear();
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Kondisi awal relay mati (HIGH)
  digitalWrite(RelayPompa, HIGH);
  digitalWrite(RelayPompa2, HIGH);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 4000) {
    lastMsg = now;
    
    // 1. Membaca semua data sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int nilaiADCTanah = analogRead(PinSoil);
    bool isRaining = (digitalRead(SensorHujan) == LOW); // LOW berarti hujan
    bool isMotion = (digitalRead(PIR) == HIGH);

    // Baca sensor ultrasonik (mentah)
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long durasi = pulseIn(echo, HIGH);
    float jarakMentah = (durasi * 0.0343) / 2;
    
    // =========================================================================
    // **IMPLEMENTASI PERSAMAAN KALIBRASI DARI SKRIPSI **
    // =========================================================================
    float kelembapanTanahPersen = (-0.0004 * nilaiADCTanah + 1.1177) * 100.0;
    float jarakTerkalibrasi = (jarakMentah - 0.284) / 1.0143;
    // =========================================================================

    // Pastikan nilai persentase tidak di bawah 0 atau di atas 100
    if (kelembapanTanahPersen < 0) kelembapanTanahPersen = 0;
    if (kelembapanTanahPersen > 100) kelembapanTanahPersen = 100;

    // 2. Logika Pengambilan Keputusan
    
    // Logika Pompa 1 (Irigasi)
    if (isRaining) {
      digitalWrite(RelayPompa, HIGH); // Mati jika hujan
      StatusP1 = "Off";
      StatusR1 = "Yes";
    } else {
      StatusR1 = "No";
      if (kelembapanTanahPersen < soil_dry_threshold_percent) { // Cek berdasarkan persentase
        digitalWrite(RelayPompa, LOW); // Nyalakan pompa
        StatusP1 = "On";
      } else {
        digitalWrite(RelayPompa, HIGH); // Matikan pompa
        StatusP1 = "Off";
      }
    }
    
    // Logika Pompa 2 (Reservoir)
    if (jarakTerkalibrasi > reservoir_kosong_cm) {
      digitalWrite(RelayPompa2, LOW); // Nyalakan pompa isi ulang
      StatusP2 = "On";
    } else if (jarakTerkalibrasi < reservoir_penuh_cm) {
      digitalWrite(RelayPompa2, HIGH); // Matikan jika sudah penuh
      StatusP2 = "Off";
    }

    // Logika Servo (Hama)
    if (isMotion) {
      StatusPs1 = "Yes";
      ServoAktif();
    } else {
      StatusPs1 = "No";
    }
    
    // 3. Menampilkan data ke Serial Monitor & LCD
    Serial.println("--------------------");
    Serial.print("Suhu: "); Serial.print(t); Serial.println(" C");
    Serial.print("Kelembapan Udara: "); Serial.print(h); Serial.println(" %");
    Serial.print("Kelembapan Tanah: "); Serial.print(kelembapanTanahPersen, 1); Serial.println(" %");
    Serial.print("Kondisi Hujan: "); Serial.println(StatusR1);
    Serial.print("Status Pompa 1: "); Serial.println(StatusP1);
    Serial.print("Jarak Reservoir: "); Serial.print(jarakTerkalibrasi, 1); Serial.println(" cm");
    Serial.print("Status Pompa 2: "); Serial.println(StatusP2);
    Serial.print("Deteksi Hama: "); Serial.println(StatusPs1);
    
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Suhu: "); lcd.print(t, 1); lcd.print("C");
    lcd.setCursor(11, 0); lcd.print("Air: "); lcd.print(jarakTerkalibrasi, 0); lcd.print("cm");
    lcd.setCursor(0, 1); lcd.print("Hum: "); lcd.print(h, 1); lcd.print("%");
    lcd.setCursor(11, 1); lcd.print("Hama: "); lcd.print(StatusPs1);
    lcd.setCursor(0, 2); lcd.print("Soil: "); lcd.print(kelembapanTanahPersen, 0); lcd.print("%");
    lcd.setCursor(11, 2); lcd.print("P1: "); lcd.print(StatusP1);
    lcd.setCursor(0, 3); lcd.print("Hujan: "); lcd.print(StatusR1);
    lcd.setCursor(11, 3); lcd.print("P2: "); lcd.print(StatusP2);

    // 4. Publikasi data ke MQTT
    char buffer[10];
    dtostrf(t, 4, 1, buffer); client.publish("OUT_ARRASYID/SUHU/T", buffer);
    dtostrf(h, 4, 1, buffer); client.publish("OUT_ARRASYID/SUHU/H", buffer);
    dtostrf(kelembapanTanahPersen, 4, 1, buffer); client.publish("OUT_ARRASYID_SOIL", buffer);
    dtostrf(jarakTerkalibrasi, 4, 1, buffer); client.publish("OUT_ARRASYID_JARAK", buffer);
    client.publish("OUT_ARRASYID_RAIN", StatusR1.c_str());
    client.publish("OUT_ARRASYID_PIR", StatusPs1.c_str());
    client.publish("OUT_ARRASYID_POMPA", StatusP1.c_str());
    client.publish("OUT_ARRASYID_KERAN", StatusP2.c_str());
  }
}
