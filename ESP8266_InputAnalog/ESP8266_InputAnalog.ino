int analog = 0;

void setup() {
  
  Serial.begin(115200);
  pinMode(D4,OUTPUT);

}
void loop() {
  analog = analogRead(A0); //baca analog dari Pin A0
  Serial.println(analog); //kirim data analog ke Serial
  if (analog>500) //Jika analog diatas 500
    digitalWrite(D4, 1); //Relay On
  else //Jika tidak
    digitalWrite(D4, 0); //Relay Off
  delay(100);
}
