#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DFRobot_DHT11.h>
#include <MQUnifiedsensor.h>

DFRobot_DHT11 DHT;
#define DHT11_PIN D5

const char* ssid = "Dejavu";
const char* password = "BISMILLAH";
const char* mqtt_server = "192.168.18.65";  // IP laptop/PC yang menjalankan broker MQTT

// MQ135 Setup
#define placa "ESP8266"
#define Voltage_Resolution 3.3
#define pin A0
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

String roundToDecimal(float value, int decimalPlaces) {
  char buffer[10];
  dtostrf(value, 0, decimalPlaces, buffer);
  return String(buffer);
}

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi terkoneksi");
  Serial.print("IP ESP8266: ");
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


}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("terhubung");
      client.subscribe("iot/monitoring"); // subscribe setelah connect
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // sensor DH11
  DHT.read(DHT11_PIN);
  float suhu = DHT.temperature;
  float kelembaban = DHT.humidity;
  
  // Sensor gas
    MQ135.update();
    float gasPPM[3] = {0};

    for (int i = 0; i < 3; i++) {
      int idx = gasTerpilih[i];
      MQ135.setA(gasA[idx]);
      MQ135.setB(gasB[idx]);
      gasPPM[i] = MQ135.readSensor();
      Serial.print(jenisgas[idx]); Serial.print(": "); Serial.print(gasPPM[i]); Serial.println(" PPM");

      delay(100);
    }

  if (!isnan(suhu) && !isnan(kelembaban) && !isnan(gasPPM[0]) && !isnan(gasPPM[1]) && !isnan(gasPPM[2])) {
    // Format JSON
    String payload = "{";
    payload += "\"suhu\": " + String(suhu, 1) + ",";
    payload += "\"kelembaban\": " + String(kelembaban, 1) + ",";
    payload += "\"CO\": " + roundToDecimal(gasPPM[0], 2) + ",";
    payload += "\"CO2\": " + roundToDecimal(gasPPM[1], 2) + ",";
    payload += "\"NH4\": " + roundToDecimal(gasPPM[2], 2);
    payload += "}";
    client.publish("iot/monitoring", payload.c_str());
    Serial.println("Data Dikirim: " + payload);
  } else {
    Serial.println("Gagal membaca data Sensor!");
  }

  delay(5000); // Kirim tiap 5 detik
}
