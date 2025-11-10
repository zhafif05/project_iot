#include <Servo.h>
Servo myservo;
int data,deg;
void setup() {
Serial.begin(115200);
myservo.attach(D4,500,2500);//set pin dan sudut 0-180
}
void loop() {
data=analogRead(A0);
deg=map(data,1024,0,0,180);//ubah skala 0-1023 ke 0-180
Serial.println(deg);
myservo.write(deg);
delay(100);
}
