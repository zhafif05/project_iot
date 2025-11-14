
#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN D1

#define RELAY_PIN D4
#define LED_PIN   D3

//Include the library
#include <MQUnifiedsensor.h>

//Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//#define calibration_button 13 //Pin to calibrate your sensor

//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

char jenisgas[6][10] = {"CO","Alcohol","CO2","Tolueno","NH4","Aceton"};
float gasA[6] = {605.18, 77.255, 110.47, 44.947, 102.2, 34.668};
float gasB[6] = {-3.937, -3.18, -2.862, -3.445, -2.473};


int gasTerpilih[] = {0,1,4}; // CO, CO2, NH4
int jumlahGas = sizeof(gasTerpilih) / sizeof(gasTerpilih[0]);

// Ambang batas
float ambangGasPPM = 100; // nilai PPM gas berbahaya
float ambangSuhu = 30.0;  // suhu dalam derajat Celsius



void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port

  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937  
  Alcohol  | 77.255 | -3.18 
  CO2      | 110.47 | -2.862
  Tolueno  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Acetona  | 34.668 | -3.369
  */
  
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ135.init(); 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ135.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  
  //MQ135.serialDebug(false);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  
}

void loop() {

  
  bool gasBerbahaya = false;

  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin

  for (int i = 0; i < jumlahGas; i++) {
    int index = gasTerpilih[i];
    MQ135.setA(gasA[index]);
    MQ135.setB(gasB[index]);

    float hasil = MQ135.readSensor();
    Serial.print(jenisgas[index]);
    Serial.print(" : ");
    Serial.print(hasil);
    Serial.println(" PPM");

    delay(1000);

    
    if (hasil >= ambangGasPPM) {
      gasBerbahaya = true;
    }

    delay(500);

  }

      if (gasBerbahaya) {
    digitalWrite(LED_PIN, LOW);  // Nyalakan LED jika gas berbahaya
    } else {
      digitalWrite(LED_PIN, HIGH);
    }

    DHT.read(DHT11_PIN);
  Serial.print("temp:");
  Serial.println(DHT.temperature);
  Serial.print("humi:");
  Serial.println(DHT.humidity);
  delay(1000);

  bool suhuTinggi = DHT.temperature >= ambangSuhu;

  // Nyalakan relay (kipas) jika suhu tinggi atau gas berbahaya
  if (gasBerbahaya || suhuTinggi) {
    digitalWrite(RELAY_PIN, LOW); // kipas ON
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // kipas OFF
  }

  delay(1000);

}