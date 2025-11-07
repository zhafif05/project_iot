int baca1,baca2,nilai,x;

void setup() {
pinMode(D4, OUTPUT);
pinMode(D2, OUTPUT); //Set Pin D4 sbg Output buzzer
pinMode(D3, INPUT_PULLUP);
pinMode(D1, INPUT_PULLUP);

}
void loop() {

baca1=digitalRead(D1);

if (baca1==0){
  
baca2=digitalRead(D3);//baca pin-D1 simpan di var baca

if (baca2==0){ //jika var baca logic 0 artinya terang
digitalWrite(D2, LOW);//hidupkan buzzer
}
else{ //selain itu //jika tidak logic 0 artinya gelap
digitalWrite(D2, HIGH); //matikan buzzer
}

digitalWrite(D4, LOW );
delay(100);
digitalWrite(D4, HIGH );
delay(100);
}

else{ 
digitalWrite(D4, HIGH);
}

}
