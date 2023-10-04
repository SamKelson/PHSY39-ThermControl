void setup() {
  pinMode(3, OUTPUT);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}

void loop() {
  
  if (Serial.available() > 0) {
    // read the incoming byte:
    int incomingByte = Serial.parseInt();

    analogWrite(3, incomingByte);
  }
}
