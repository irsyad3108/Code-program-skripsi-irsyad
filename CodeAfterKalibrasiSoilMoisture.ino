after kalbrasi soil
/*
  Program untuk Membaca Sensor Kelembaban Tanah dengan ESP32
  - Menggunakan fungsi kalibrasi linear dari hasil regresi
*/

// Tentukan pin GPIO tempat sensor terhubung.
const int pin_sensor = 34;

void setup() {
  // Mulai komunikasi serial pada baud rate 115200
  Serial.begin(115200);
  Serial.println("Membaca Sensor Kelembaban Tanah (Model Linear)...");
}

void loop() {
  // Baca nilai analog mentah dari sensor
  float nilai_sensor = analogRead(pin_sensor);

  // --- Implementasi Rumus Linear dari Grafik ---
  // Persamaan: y = -0.0004x + 1.1177
  double m = -0.0004; // Koefisien x (slope)
  double c = 1.1177;  // Konstanta (intercept)

  // Hitung kelembapan dalam format desimal (y)
  double kelembapan_desimal = (m * nilai_sensor) + c;

  // Ubah dari desimal ke persentase
  double kelembapan_persen = kelembapan_desimal * 100;

  // Pastikan nilai persentase tidak keluar dari rentang 0-100
  kelembapan_persen = constrain(kelembapan_persen, 0, 100);

  // Tampilkan hasilnya di Serial Monitor
  Serial.print("Nilai Sensor Mentah: ");
  Serial.print(nilai_sensor);
  Serial.print("  |  Kelembaban Terkalibrasi: ");
  Serial.print(kelembapan_persen, 2); // Tampilkan 2 angka di belakang koma
  Serial.println("%");

  // Beri jeda 1 detik sebelum pembacaan berikutnya
  delay(1000);
}