// August 19, 2023
//Works!

// Reads voltage from voltage divider with thermistor as one leg. Averages numMeasurements times.
// Thermistor is in leg next to ground and the voltage is read by "tempIn"
// The thermistor is a Epcos, R/T 1008 with R_25 = ~2000 ohms. B-parameter equation given in code.
// Outputs temperature (C), elapsed time (s) and PWM (0-255)
// Example serial line output:
//              Temperature (C): 27.73, Time (s): 645.06, PWM: 127

// Works with Matlab app "testappV2"
// Matlab reads and plots the results in a stripchart
// Matlab sets pwmValue between [-255,255] and passes value to Arduino via Serial
// Matlab sets HeatCool as either 0 (cool) or 1 (heat) and passes value to Arduino via Serial

// power
#define pwmOut1 9
#define pwmOut2 10

// Fixed resistor value in ohms
// const float fixedResistance = 4670.0; // my board
const float fixedResistance = 1700.0; // board in 39 lab

// Thermistor parameters for 100k ohm 
const float beta = 3560.0;
const float R25 = 2000.0;
const float T0 = 298.15;  // 25°C in Kelvin

const int tempIn = A0;  // Analog input pin
const int numMeasurements = 5000;  // Number of measurements



//Set running mode: 0:manual, 1:p, 2:p/i
int mode = 0;

//set temperature from matlab
float setTemp = 20;

//running?
int onOff = 0;

//constants for p and p/i
float ki = 0;
float kp = 0;

float error = 0;
float integral = 0;

int  pwmValue = 0;
int  HeatCool = 1;

unsigned long startTime = 0;

float prevTime = 0;

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  while (!Serial) {
    ;  // Wait for serial port to connect
  }
  startTime = millis();  // Get the starting time
}

void loop() {

  //Look for commands from matlab
 if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');

    if (command.startsWith("Heat/Cool")) {
      sscanf(command.c_str(), "Heat/Cool %d", &HeatCool);
    }
    
    if (command.startsWith("PWM")) {
      sscanf(command.c_str(), "PWM %d", &pwmValue);
    }

    if (command.startsWith("Integral")) {
      char * strtokIndx;
      strtokIndx = strtok(command.c_str()," ");
      strtokIndx = strtok(NULL, ",");
      integral = atof(strtokIndx);
    }

    if (command.startsWith("Mode")) {
      sscanf(command.c_str(), "Mode %i", &mode);
    }

    if (command.startsWith("On/Off")) {
      sscanf(command.c_str(), "On/Off %i", &onOff);
    }

    if (command.startsWith("kp")) {
      
      char * strtokIndx;
      strtokIndx = strtok(command.c_str()," ");
      strtokIndx = strtok(NULL, ",");
      kp = atof(strtokIndx);

    }

    if (command.startsWith("ki")) {

      char * strtokIndx;
      strtokIndx = strtok(command.c_str()," ");
      strtokIndx = strtok(NULL, ",");
      ki = atof(strtokIndx);

    }

    if (command.startsWith("SetTemperature")) {

      char * strtokIndx;
      strtokIndx = strtok(command.c_str()," ");
      strtokIndx = strtok(NULL, ",");
      setTemp = atof(strtokIndx);

    }
  }

  //collect temp: Perform measurements and calculate average
  float total = 0;
    for (int i = 0; i < numMeasurements; i++) {
      int sensorValue = analogRead(tempIn);
      total = total + sensorValue;
    }
  float average = total / (float)numMeasurements;

  // Convert the analog value to voltage
  float voltage = average * (5.0 / 1023.0);  // Assuming 5V reference voltage

  // Calculate the resistance of the thermistor
  float thermistorResistance = (fixedResistance * voltage) / (5.0 - voltage);

  // Calculate the temperature using the Steinhart-Hart equation. See Wiki Thermistor entry.
  float steinhart = log(thermistorResistance / R25);
  steinhart /= beta;
  steinhart += 1.0 / T0;
  float temperature = 1.0 / steinhart - 273.15;  // temperature in Centigrade

  error = setTemp - temperature;

  float elapsedTime = (millis() - startTime) / 1000.0;

  //Perform calculations

  if(onOff == 1){

    if(mode==0){

      //do nothing, PWM is set by user

    }else if(mode==1){

      // proportional control  (P)
      pwmValue = round(min(max(kp * abs(error), 0), 255));

      // heat or cool
      if (error > 0){ // heat
        HeatCool = 1;
      }else if(error <= 0){ // cool
        HeatCool = 0;
      } 

    }else if(mode==2){

      // proportional integral control (PI)
      
      float dt = elapsedTime; // time interval for last measurement
      integral = integral + ((elapsedTime - prevTime) * error);  // update integral of error
      float u = kp * error + ki * integral;  // calculate PI feedback
      pwmValue = min(abs(round(u)),255);  // magnitude of PI feedback term u limited to a max of 255 for arduino PWM

      //turned off when testing KI overshoot
      //if(ki * integral>255){integral=0;}

      // heat or cool
      if (u > 0){ // heat
        HeatCool = 1;
      }else if(u <= 0){ // cool
        HeatCool = 0;
      } 

    }

    //Update state based on incoming commands and/or results of p, p/i

    prevTime = elapsedTime;

    if (HeatCool == 1){
      analogWrite(pwmOut1, abs(pwmValue));
      analogWrite(pwmOut2,0); 
    }
    if (HeatCool == 0){
      analogWrite(pwmOut2, abs(pwmValue));
      analogWrite(pwmOut1,0); 
    }

  }else{
    analogWrite(pwmOut2, 0);
    analogWrite(pwmOut1, 0);
  }

  // send data to matlab
  Serial.print("Temperature (C): ");
  Serial.print(temperature);

  Serial.print(", Time (s): ");
  Serial.print(elapsedTime);

  Serial.print(", Heat/Cool: ");
  Serial.print(HeatCool);

  Serial.print(", PWM: ");
  Serial.print(pwmValue);

  Serial.print(", Error: ");
  Serial.print(error);

  Serial.print(", Integral: ");
  Serial.print(integral);

  Serial.print(", Mode: ");
  Serial.print(mode);

  Serial.print(", On: ");
  Serial.print(onOff);

  Serial.print(", kp: ");
  Serial.print(kp);

  Serial.print(", ki: ");
  Serial.print(ki);

  Serial.print(", setTemp: ");
  Serial.println(setTemp);
}