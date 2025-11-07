int baca, nilai,x; //buat variable baca sensor
String text="";
void setup(){
Serial.begin(115200); //Aktifkan Serial di baudrate (bps)
pinMode(D2, OUTPUT); //Set Pin D2 sbg Output buzzer
pinMode(D3, INPUT_PULLUP); //Set Pin D3 sbg Input LDR
}
void loop() {
baca=digitalRead(D3);//baca pin-D3 simpan di var baca
if (baca==1){ //jika var baca logic 1 artinya ada bayangan
digitalWrite(D2, HIGH);//hidupkan buzzer
Serial.println("ada bayangan"); //kirim test ke serial
} //jika tidak logic 0 artinya gelap
else{ //selain itu
digitalWrite(D2, LOW); //matikan buzzer
}
if(Serial.available()){ // jika ada serial masuk
delay(10); text="";
while(Serial.available()){
text+=(char)Serial.read(); // baca isi serial
}
nilai=text.toInt();//ubah dari text jadi angka
Serial.print("setting buzzer: ");//kirim ke serial
Serial.println(nilai);
if (nilai>0){
digitalWrite(D2,LOW); delay(nilai); //buzzer aktif
}else{
digitalWrite(D2,HIGH); delay(5000); //buzzer mati 5s
}
}
}
