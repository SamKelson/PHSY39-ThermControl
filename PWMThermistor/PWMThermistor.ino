double volts = 0.0;
int i = 0;
unsigned long timeStarted = millis();

void setup() {
  pinMode(A0,INPUT);
  pinMode(3, OUTPUT);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}

void loop() {
  unsigned long time = (millis() - timeStarted);

  double voltThermistor = 5 * (analogRead(A0)/1023.0);

  volts += voltThermistor;

  if(i>=1000){
    Serial.print("{");
    Serial.print("\"time\": ");
    Serial.print(time);
    Serial.print(", \"avgVolt\": ");
    Serial.print(volts/1000);
    Serial.print("}");
    Serial.println();

    analogWrite(3, 255.0*(voltThermistor/5));

    i=0;
    volts=0;
  }
  i++;
}
