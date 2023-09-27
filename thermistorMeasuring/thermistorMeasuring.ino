double temps = 0.0;
int i = 0;  

void setup() {
  pinMode(A0,INPUT);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}

void loop() {
  double voltThermistor = 5 * (analogRead(A0)/1023.0);

  double current = (5-voltThermistor)/1700;

  double resitance = voltThermistor/current;

  double B = 3560.;
  double rInf = 2000.0 * exp(-B/298.0);
  double T = B/log(resitance/rInf) - 273;

  //Serial.println(resitance);

  temps += T;

  if(i>=1000){
    Serial.println(temps/1000);

    i=0;
    temps=0;
  }
  i++;
}