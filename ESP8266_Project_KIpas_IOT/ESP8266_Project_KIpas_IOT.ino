/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"

#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN D1

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "BBUS-D1J3Efql1DPoTOhHrL4MqHNoDiHPsD"  // Your Ubidots TOKEN
#define WIFINAME "Dejavu"                            // Your SSID
#define WIFIPASS "BISMILLAH"                         // Your Wifi Pass

const int relay = D4;       // Penulisan D4 menjadi led
bool manualOverride = false; // Status kontrol manual dari Ubidots
unsigned long lastUbidotsInput = 0; // Waktu terakhir menerima input dari Ubidots

Ubidots client(TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Kontrol relay berdasarkan input dari Ubidots
  if ((char)payload[0] == '1') {
    digitalWrite(relay, LOW); // Relay mati
    manualOverride = true;    // Aktifkan kontrol manual
    lastUbidotsInput = millis(); // Simpan waktu terakhir input
  } else {
    digitalWrite(relay, HIGH); // Relay menyala
    manualOverride = true;
    lastUbidotsInput = millis();
  }
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nMemulai program...");

  pinMode(relay, OUTPUT);

  client.setDebug(true);  // Debug mode
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);

  client.ubidotsSubscribe("esp8266_z", "relay");  // Subscribe ke Ubidots
  Serial.println("Sistem siap. Ketik 'enable' atau 'disable' di Serial.");
}

void loop() {
    // Baca data sensor
    DHT.read(DHT11_PIN);
    client.add("suhu", DHT.temperature);
    client.add("kelembapan", DHT.humidity);
    client.ubidotsPublish("esp8266_z");

    // Logika kontrol relay
    unsigned long currentTime = millis();

    // Cek apakah kontrol manual masih berlaku
    if (manualOverride && (currentTime - lastUbidotsInput > 30000)) { // 30 detik timeout
      manualOverride = false; // Reset kontrol manual jika timeout
    }

    // Jika tidak dalam mode manual, kontrol relay berdasarkan kelembapan
    if (!manualOverride) {
      if (DHT.humidity >= 80) {
        digitalWrite(relay, LOW); // Relay mati
      } else if (DHT.humidity <= 80) {
        digitalWrite(relay, HIGH); // Relay menyala
      }else if (DHT.humidity <= 40) {
        digitalWrite(relay, HIGH); // Relay menyala
      }
    }

    client.loop();
    delay(5000); 
  }

 
