#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <MQUnifiedsensor.h>
#include <DFRobot_DHT11.h>
#include <NewPing.h>

DFRobot_DHT11 DHT;
#define DHT11_PIN D5
int relay = D1; 
int led = D4;
// WiFi Setup
const char* ssid = "werrr";
const char* password = "anjayani";

// Endpoint Server
const char* server = "http://192.168.94.114/iot-dashboard/insert.php";

// MQ135 Setup
#define placa "ESP8266"
#define Voltage_Resolution 3.3
#define pin D3
#define type "MQ-135"
#define ADC_Bit_Resolution 10
#define RatioMQ135CleanAir 3.6

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// Parameter Gas
char jenisgas[6][10] = {"CO", "Alcohol", "CO2", "Tolueno", "NH4", "Aceton"};
float gasA[6] = {605.18, 77.255, 110.47,24.947, 102.2, 34.668};
float gasB[6] = {-3.937, -3.18, -2.862, -3.445, -2.473, -3.369};
int gasTerpilih[] = {0, 2, 4}; // CO, CO2, NH4

float ambangGasPPM = 100;
float ambangSuhu = 80.0;


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Mulai koneksi WiFi...");

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {
      Serial.println("\n❌ Gagal konek WiFi. Cek SSID/Password.");
      return;
    }
  }

  Serial.println("\n✅ WiFi Terkoneksi!");
  Serial.print("📡 IP Address: ");
  Serial.println(WiFi.localIP());

  // Kalibrasi MQ135
  MQ135.init();
  MQ135.setRegressionMethod(1);
  Serial.print("🔧 Kalibrasi sensor MQ135... ");
  float r0 = 0;
  for (int i = 0; i < 10; i++) {
    MQ135.update();
    r0 += MQ135.calibrate(RatioMQ135CleanAir);
    delay(300);
  }
  r0 /= 10;
  MQ135.setR0(r0);
  Serial.println("✅ Selesai kalibrasi!");

  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(relay,LOW);
}

void loop() {
  bool gasBerbahaya = false;

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    // Simulasi suhu & kelembapan
    DHT.read(DHT11_PIN);
    float suhu = DHT.temperature;
    float kelembapan = DHT.humidity;


    // Sensor gas
    MQ135.update();
    float gasPPM[3] = {0};

    for (int i = 0; i < 3; i++) {
      int idx = gasTerpilih[i];
      MQ135.setA(gasA[idx]);
      MQ135.setB(gasB[idx]);
      gasPPM[i] = MQ135.readSensor();
      Serial.print(jenisgas[idx]); Serial.print(": "); Serial.print(gasPPM[i]); Serial.println(" PPM");

      if (gasPPM[i] >= ambangGasPPM) {
        gasBerbahaya = true;
      }

      delay(100);
    }

    
  // Baca suhu dari sensor DHT11
  DHT.read(DHT11_PIN);
  bool suhuTinggi = DHT.humidity >= ambangSuhu;

  // Tindakan berdasarkan sensor
  if (gasBerbahaya) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }

  if (gasBerbahaya || suhuTinggi) {
    digitalWrite(relay, HIGH);
    delay(10000);
  } else {
    digitalWrite(relay, LOW);
  }

  int relayStatus = digitalRead(relay) == HIGH ? 1 : 0; // 1 = ON, 0 = OFF

    // Buat JSON payload
    StaticJsonDocument<256> doc;
    doc["esp_suhu"] = suhu;
    doc["esp_kelembapan"] = kelembapan;
    doc["co"] = roundToDecimal(gasPPM[0],2);
    doc["co2"] = roundToDecimal(gasPPM[1],2);
    doc["nh4"] = roundToDecimal(gasPPM[2],2);
    doc["relay_status"] = relayStatus;  


    String jsonString;
    serializeJson(doc, jsonString);

    http.begin(client, server);
    http.addHeader("Content-Type", "application/json");

    Serial.println("\n🌐 Kirim POST ke: " + String(server));
    Serial.println("📦 Payload JSON: " + jsonString);

    int httpCode = http.POST(jsonString);

    if (httpCode > 0) {
      Serial.print("📬 HTTP Response Code: ");
      Serial.println(httpCode);
      Serial.print("📨 Response: ");
      Serial.println(http.getString());
    } else {
      Serial.print("❗ Gagal kirim. Kode: ");
      Serial.println(httpCode);
    }


    int httpCode2 = http.GET();
  if (httpCode2 == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(256);
    deserializeJson(doc, payload);
    int relay_status = doc["relay_status"];
    digitalWrite(relay, relay_status);
  }

    http.end();
  } else {
    Serial.println("⚠ WiFi tidak terhubung!");
  }

  delay(5000);
}
