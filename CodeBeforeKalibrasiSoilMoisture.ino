before kalibrasi soil

/*
  Program untuk Membaca Sensor Kelembaban Tanah dengan ESP32
  - Menggunakan kalibrasi linear (map) yang stabil sesuai data grafik
*/

// --- PENGATURAN KALIBRASI (SESUAIKAN DENGAN DATA GRAFIK ANDA) ---
#define NILAI_SENSOR_KERING 2750 // Nilai ADC saat kelembapan 0%
#define NILAI_SENSOR_BASAH 1720  // Nilai ADC saat kelembapan 40%

// Tentukan pin GPIO tempat sensor terhubung.
const int pin_sensor = 34;

void setup() {
  // Mulai komunikasi serial pada baud rate 115200
  Serial.begin(115200);
  Serial.println("Membaca Sensor Kelembaban Tanah...");
}

void loop() {
  // Baca nilai analog mentah dari sensor
  int nilai_sensor = analogRead(pin_sensor);

  // Konversi nilai mentah ke persentase menggunakan map()
  // Memetakan rentang ADC (2750 -> 1720) ke rentang persen (0% -> 40%)
  int kelembaban_persen = map(nilai_sensor, NILAI_SENSOR_KERING, NILAI_SENSOR_BASAH, 0, 40);

  // Pastikan nilai persentase tidak keluar dari rentang 0-100
  kelembaban_persen = constrain(kelembaban_persen, 0, 100);

  // Tampilkan hasilnya di Serial Monitor
  Serial.print("Nilai Sensor Mentah: ");
  Serial.print(nilai_sensor);
  Serial.print("  |  Kelembaban: ");
  Serial.print(kelembaban_persen);
  Serial.println("%");

  // Beri jeda 1 detik sebelum pembacaan berikutnya
  delay(1000);
}