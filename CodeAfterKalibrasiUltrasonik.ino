/*
 * Program untuk mengukur jarak menggunakan sensor ultrasonik HC-SR04
 * dan menampilkannya di Serial Monitor dengan kalibrasi.
 */

// Mendefinisikan pin untuk Trig dan Echo
const int trigPin = 26;
const int echoPin = 27;

// Variabel untuk menyimpan durasi dan jarak
long durasi;
float jarak; // Diubah ke float untuk presisi
float jarakTerkalibrasi; // Variabel baru untuk jarak hasil kalibrasi

void setup() {
  // Mengatur trigPin sebagai OUTPUT dan echoPin sebagai INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Memulai komunikasi serial pada 9600 baud
  Serial.begin(9600);
  Serial.println("Pengukuran Jarak dengan Sensor Ultrasonik (Terkalibrasi)");
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
  
  // Menghitung jarak mentah (sebagai 'x' dalam rumus)
  jarak = durasi * 0.0343 / 2;

  // Menerapkan rumus kalibrasi
  jarakTerkalibrasi = (1.0143 * jarak) + 0.284;

  // Variabel untuk persentase
  int kosong = 31;
  int penuh = 6;
  
  // Menghitung persentase menggunakan jarak yang sudah dikalibrasi
  int persentase = map(jarakTerkalibrasi, kosong, penuh, 0, 100);

  // Menampilkan hasil pengukuran yang sudah dikalibrasi ke Serial Monitor
  Serial.print("Jarak: ");
  Serial.print(jarakTerkalibrasi); // Menampilkan jarak hasil kalibrasi
  Serial.println(" cm");
  delay(1000);
}