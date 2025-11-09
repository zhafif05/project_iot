int baca, nilai,x; //buat variable baca sensor
String text="";
int led=D4;

void setup() {
Serial.begin(115200);
pinMode(led,OUTPUT);
}

void loop() {
if(Serial.available()){ delay(10); text="";//serial masuk
while(Serial.available()){
text+=(char)Serial.read(); // baca isi serial
}
nilai=text.toInt(); //ubah data masuk dari text ke angka
Serial.println(nilai);
//1023-nilai -> pembalik kondisi LED_Builtin aktif low
analogWrite(led,1023-nilai);//0=padam,200=redup,1023 Max
}
delay(100);
}
