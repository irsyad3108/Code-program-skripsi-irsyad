/*
 * Program untuk mengukur jarak menggunakan sensor ultrasonik HC-SR04
 * dan menampilkannya di Serial Monitor dengan menerapkan rumus kalibrasi
 * dari skripsi untuk mengoreksi pembacaan sensor.
 */

// Mendefinisikan pin untuk Trig dan Echo sesuai proyek Anda
const int trigPin = 26;
const int echoPin = 27;

// Variabel untuk menyimpan durasi dan jarak
long durasi;
float jarakMentah;        // Variabel untuk jarak mentah sebelum dikoreksi
float jarakTerkalibrasi;  // Variabel untuk jarak setelah dikoreksi dengan rumus

void setup() {
  // Mengatur pin Trig sebagai OUTPUT dan pin Echo sebagai INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Memulai komunikasi serial pada 9600 baud
  Serial.begin(9600);
  Serial.println("--- Uji Kalibrasi Sensor Ultrasonik HC-SR04 ---");
}

void loop() {
  // 1. Memicu sensor untuk mengirimkan gelombang suara
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 2. Membaca durasi pantulan sinyal dari pin Echo
  durasi = pulseIn(echoPin, HIGH);
  
  // 3. Menghitung jarak mentah (ini adalah 'y' dalam rumus)
  jarakMentah = durasi * 0.0343 / 2;

  // 4. Menerapkan RUMUS TERBALIK untuk mengoreksi dan mendapatkan jarak sebenarnya ('x')
  // Berdasarkan rumus dari skripsi: y = 1.0143x + 0.284
  // Maka, x = (y - 0.284) / 1.0143
  jarakTerkalibrasi = (jarakMentah - 0.284) / 1.0143;

  // 5. Menampilkan kedua hasil ke Serial Monitor untuk perbandingan
  Serial.print("Jarak Mentah (Sensor): ");
  Serial.print(jarakMentah);
  Serial.print(" cm | Jarak Terkalibrasi (Hasil Sebenarnya): ");
  Serial.print(jarakTerkalibrasi);
  Serial.println(" cm");
  
  delay(1000); // Jeda 1 detik agar mudah dibaca
}
