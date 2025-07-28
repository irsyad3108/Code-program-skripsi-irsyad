#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // libray lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);



#define DHTTYPE DHT11
#define dht_pin 33
#define PIR 12
#define PinSoil 34
#define SensorHujan 15
#define trig 26
#define echo 27 

#define RelayPompa 5
#define RelayPompa2 18


#define IN_TOPIC "IN_ARRASYID"
#define IN_TOPIC2 "IN_ARRASYID_TOMBOL_POMPA"
#define IN_TOPIC3 "IN_ARRASYID_TOMBOL_KERAN"

//#define OUT_TOPIC "OUT_ARRASYID"

String StatusR1;
String StatusP1;
String StatusPs1;
String StatusP2;

const int dry = 3700;
const int mid_dry = 1250;
const int wet = 125; 

const int kosong = 32;
const int penuh = 6;

// const char* ssid = "AYONA";
// const char* password = "Arrasyid21";
// const char* ssid = "@UPI.EDU WPA";
// const char* password = "Pendidikan"; 
const char* ssid = "Zuppa";
const char* password = "Ujang123";

const char* mqtt_server = "broker.hivemq.com";

DHT dht(dht_pin,DHTTYPE);
Servo servo1;



WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int val = 0;
int pos = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if((char)payload[0] == 'a'){
    digitalWrite (RelayPompa2, LOW);
    client.publish("OUT_ARRASYID_KERAN", "On");
    Serial.println ("Pompa 2 : On");
  }else if((char)payload[0] == 'b'){
    digitalWrite (RelayPompa2, HIGH);
    client.publish("OUT_ARRASYID_KERAN", "Off");
    Serial.println ("Pompa 2 : Off");
  }else if((char)payload[0] == 'p'){
    digitalWrite(RelayPompa,LOW);
    client.publish("OUT_ARRASYID_POMPA", "On");
    Serial.println ("Pompa 1 : On");
  }else if((char)payload[0] == 'q'){
    digitalWrite(RelayPompa,HIGH);
    client.publish("OUT_ARRASYID_POMPA", "Off");
    Serial.println ("Pompa 1 : Off");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(IN_TOPIC);
      client.subscribe(IN_TOPIC2);
      client.subscribe(IN_TOPIC3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ServoAktif() {
  // put your main code here, to run repeatedly:
  for (pos = 0; pos <= 180; pos += 30) { // servo bergerak dari posisi 0 drajat menuju 180 drajat
    // in steps of 1 degree
    servo1.write(pos);              // Menyuruh servo bergerak menuju posisi sesuai variabel 'pos'
    delay(15);                       
  }
  for (pos = 180; pos >= 0; pos -= 30) { // servo bergerak dari posisi 0 drajat menuju 180 drajat
    servo1.write(pos);              // Menyuruh servo bergerak menuju posisi sesuai variabel 'pos'
    delay(15);                       
  }
}


void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(PIR, INPUT);
  pinMode(PinSoil, INPUT);
  pinMode (SensorHujan, INPUT);
  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT);

  pinMode (RelayPompa, OUTPUT);
  pinMode(RelayPompa2, OUTPUT);
  servo1.attach(2);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  digitalWrite (RelayPompa, HIGH);
  digitalWrite (RelayPompa2, HIGH);

  // --- PERBAIKAN DI SINI ---
  // Menggunakan init() dan backlight() untuk inisialisasi LCD
  // yang lebih kompatibel dengan ESP32.
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0,1);
  lcd.print("  Smart Irrigation  ");
  lcd.setCursor(8,2);
  lcd.print("System");
  delay(2000);
  lcd.clear();
}

void loop() {
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  int SensorSoil = analogRead(PinSoil); //637-638 272-273
  int PercentageSensorSoil = map(SensorSoil, wet, dry, 100, 17);

  int DataSensorHujan = digitalRead(SensorHujan);

  val = digitalRead(PIR);

  digitalWrite(trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trig, LOW);

  long durasi;
  float jarak; // Gunakan float untuk presisi
  durasi = pulseIn(echo, HIGH);
  jarak = (durasi * 0.0343) / 2; 
  int PercentageJarak = map(jarak, kosong, penuh, 0, 100);
  
  if (!client.connected()) {                                 
    reconnect();
  }
  client.loop();                                               

  unsigned long now = millis();
  if (now - lastMsg > 4000) {                                  
    lastMsg = now;
    // Convert the value to a char array
    char tempString[8];
    dtostrf(t, 1, 0, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(tempString);
    lcd.print(" C");
    client.publish("OUT_ARRASYID/SUHU/T", tempString);
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(h, 1, 0, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    lcd.setCursor(0,1);
    lcd.print("Hum: ");
    lcd.print(humString);
    client.publish("OUT_ARRASYID/SUHU/H", humString); 

    
    Serial.print("Soil Moisture: ");
    Serial.print(PercentageSensorSoil - 25);
    Serial.println ("%");
    lcd.setCursor(0,2);
    lcd.print("SM: ");
    lcd.print(PercentageSensorSoil -25);
    lcd.print ("%");

    if(DataSensorHujan == 1 && SensorSoil >= dry){
      Serial.println("Rain : No");
      Serial.println ("Pompa 1 : On");
      digitalWrite(RelayPompa,LOW);
      StatusR1 = "No";
      StatusP1 = "On";
      client.publish("OUT_ARRASYID_POMPA", "On");
      client.publish("OUT_ARRASYID_RAIN", "Rain : No");
    }else if(DataSensorHujan == 1 && SensorSoil <= mid_dry){
      digitalWrite(RelayPompa,HIGH);
      Serial.println("Rain : No");
      Serial.println ("Pompa 1 : Off");
      StatusR1 = "No";
      StatusP1 = "Off";
      client.publish("OUT_ARRASYID_POMPA", "Off");
      client.publish("OUT_ARRASYID_RAIN", "No");
    }else if (DataSensorHujan == 0){
      Serial.println("Rain : Yes");
      Serial.println ("Pompa 1 : Off");
      digitalWrite(RelayPompa,HIGH);
      StatusR1 = "Yes";
      StatusP1 = "Off";
      client.publish("OUT_ARRASYID_POMPA", "Off");
      client.publish("OUT_ARRASYID_RAIN", "Yes");
    }
    
    snprintf (msg, MSG_BUFFER_SIZE, "%i", PercentageSensorSoil - 25);
    client.publish("OUT_ARRASYID_SOIL", msg);


    if (val == HIGH){
      Serial.println("Pest: Yes");
      StatusPs1 = "Yes";
      client.publish("OUT_ARRASYID_PIR", "Yes"); 
      ServoAktif();
    }else{
      servo1.write(0);
      Serial.println("Pest : No");
      StatusPs1 = "No";
      client.publish("OUT_ARRASYID_PIR", "No"); 
    }
    

    Serial.print ("Reservoir: ");
    Serial.print (PercentageJarak);
    Serial.println ("% ");
    lcd.setCursor(11,0);
    lcd.print("WTR :");
    lcd.print(PercentageJarak);
    lcd.print("%"); // Menggunakan print bukan println untuk LCD

    
    lcd.setCursor(0,3);
    lcd.print("Rain: ");
    lcd.print(StatusR1);
    lcd.setCursor(11,2);
    lcd.print("P1: ");
    lcd.print(StatusP1);

    lcd.setCursor(11,1);
    lcd.print("Pest: ");
    lcd.print(StatusPs1);


    if ( PercentageJarak >= 100) {
      Serial.println ("(Torn Penuh)");
      Serial.println ("Pompa 2 : Off");
      Serial.println (" ");
      digitalWrite (RelayPompa2, HIGH);
      StatusP2 = "Off";
      client.publish("OUT_ARRASYID_KERAN", "Off");
      }

    else if ( PercentageJarak <= 15) {
      Serial.println ("(Torn Kosong)");
      Serial.println ("Pompa 2 : On");
      Serial.println (" ");
      digitalWrite (RelayPompa2, LOW);
      StatusP2 = "On";
      client.publish("OUT_ARRASYID_KERAN", "On");
      }


    lcd.setCursor(11,3);
    lcd.print("P2: ");
    lcd.print(StatusP2);
    
    snprintf (msg, MSG_BUFFER_SIZE, "%i", PercentageJarak);
    client.publish("OUT_ARRASYID_JARAK", msg);
  
  }

  delay(4000);
  lcd.clear();
}