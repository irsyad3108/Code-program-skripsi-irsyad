/*
 * Program untuk mengukur jarak menggunakan sensor ultrasonik HC-SR04
 * dan menampilkannya di Serial Monitor.
 */

// Mendefinisikan pin untuk Trig dan Echo
const int trigPin = 26;
const int echoPin = 27;

// Variabel untuk menyimpan durasi dan jarak
long durasi;
int jarak;

void setup() {
  // Mengatur trigPin sebagai OUTPUT dan echoPin sebagai INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Memulai komunikasi serial pada 9600 baud
  Serial.begin(9600);
  Serial.println("Pengukuran Jarak dengan Sensor Ultrasonik");
}

void loop() {
  // Membersihkan trigPin sebelum mengirim sinyal
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Mengirim sinyal ultrasonik selama 10 mikrodetik
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Membaca durasi pantulan sinyal dari echoPin
  durasi = pulseIn(echoPin, HIGH);
  
  // Menghitung jarak berdasarkan durasi
  // Kecepatan suara = 343 m/s atau 0.0343 cm/µs
  // Rumus: Jarak = (Durasi * Kecepatan Suara) / 2
  // Dibagi 2 karena sinyal bergerak bolak-balik (sensor ke objek lalu kembali)
  jarak = durasi * 0.0343 / 2;

  // Variabel untuk persentase
  int kosong = 31;
  int penuh = 6;
  
  // Menghitung persentase menggunakan jarak mentah
  int persentase = map(jarak, kosong, penuh, 0, 100);

  // Menampilkan hasil pengukuran ke Serial Monitor
  Serial.print("Jarak: ");
  Serial.print(jarak);
  Serial.println(" cm");
  delay(1000);
}