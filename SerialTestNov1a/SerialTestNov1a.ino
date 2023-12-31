    //Works! October 31, 2023

    // There are 4 thermistors. The UNO has 6 analog input pins, so there is room for 2 more thermistors.
    // The one called "temp" is located just next to the TEC.
    // The other 3, T1, T2, T3 are located along the rod.
    
    // Expects 9 Params from Matlab in the following order and MatlabParams class:
    // MLparams MatlabParams MLparams[mode pwmNow HeatCool Kp KI Tset ResetInt SinAmp SinPer] with formats [%s %d %d %f %f %f %d %f %f]
    // Uses a String tokenizer to split the received string at spaces

    // Sends output of 10 values to Matlab as a ArduinoParams class:  
    // ArduinoParams ARDparams[temp, time, pwmNow, HeatCool, error, Integral, Tset, T1, T2, T3] with formats [%f, %f, %d, %d, %f, %f, %f, %f, %f, %f]

// For sinusoidally varying set temperatue, allow the cooling to have more power than heating
// coolToHeatfactor is the factor by which cooling exceeds heating.


unsigned long startTime = 0;
String receivedString;
String FromMatlab;

const int tempIn0 = A0;  // Analog input pin
const int tempIn1 = A1;  // Analog input pin
const int tempIn2 = A2;  // Analog input pin
const int tempIn3 = A3;  // Analog input pin
const int tempIn4 = A4;  // Analog input pin
const int tempIn5 = A5;  // Analog input pin

const int numMeasurements = 5000;  // Number of measurements

class MatlabParams {
public:
  String mode;    // Manual, Proportional or P/I
  int pwmNow;     // PWM [0 255]
  int HeatCool;   // Cool or Heat [-1 1]
  float Kp;       // proportional gain
  float KI;       // integral gain
  float Tset;     // Set temperature in centigrade
  int ResetInt;    // logical [0 1] to reset integral term in P/I
  float SinAmp;    // Amplitude of sinusoidally varying T_set
  float SinPer;    // Period in seconds of sinusoidally varying T_set

  // Constructor
  MatlabParams(String m, int pwm, int hc, float kp, float ki, float t, int rI, float sA, float sP) {
    mode = m;
    pwmNow = pwm;
    HeatCool = hc;
    Kp = kp;
    KI = ki;
    Tset = t;
    ResetInt = rI;
    SinAmp = sA;
    SinPer = sP;
  }
};

class ArduinoParams {
public:
  float temp;    // temperature
  float time;     // elapsed time
  int pwmNow;     // PWM [0 - 255]
  int HeatCool;    // Cool or Heat [-1 1]
  float error;     // Tset - temperature
  float Integral;   // integral term
  float Tset;        // set Temperature with Sin term
  float T1;       // temperature 1
  float T2;       // temperature 2
  float T3;       // temperature 3
  float T4;
  float T5;

  // Constructor
  ArduinoParams(float T, float t, int pwm, int HC, float e, float I, float Ts, float T1, float T2, float T3, float T4, float T5) {
    temp = T;
    time = t;
    pwmNow = pwm;
    HeatCool = HC;
    error = e;
    Integral = I;
    Tset = Ts;
    T1 = T1;
    T2 = T2;
    T3 = T3;
    T4 = T4;
    T5 = T5;
  }
};

// power
#define pwmOut1 9
#define pwmOut2 10

MatlabParams MLparams("Manual", 0, 1, 15.6, 0.1, 35.0, 1, 0, 60);  // Initial Matlab parameters. Set PWM = 0 for safety
ArduinoParams ARDparams(20, 0, 0, 1, 0, 0, 20, 20, 20, 20, 20, 20);  // Initial Arduino parameters. Set initial time to zero

// Fixed resistor value in ohms
// const float fixedResistance = 4670.0; // my board
const float fixedResistance = 98000.0; // board in 39 lab

// Thermistor parameters for 100k ohm 
const float beta = 4540.0;
const float R25 = 100000.0;
const float T0 = 298.15;  // 25°C in Kelvin

int  pwmValue = 0;
int  HeatCool = 1;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;  // Wait for serial port to connect
  }
    // 6 Params from Matlab
    // Mode PWM H/C Kp KI Tset with formats %s %d %d %f %f %f


  unsigned long startTime = millis();  // Get the starting time
}

void loop() {


 float T0 = measureTemperature(tempIn0, 1000); // Measure thermistor at A0 pin
 float T1 = measureTemperature(tempIn1, 1000) - 0.2; // Measure thermistor at A0 pin
 float T2 = measureTemperature(tempIn2, 1000) + 0.1; // Measure thermistor at A0 pin
 float T3 = measureTemperature(tempIn3, 1000) + 0.1; // Measure thermistor at A0 pin
 float T4 = measureTemperature(tempIn4, 1000) + 0;
 float T5 = measureTemperature(tempIn5, 1000) - 0.2;

// The ARDparams class object is sent to Matlab after every cycle of loop. Once the temperature has been measured 
// and averaged numMeasurements times, the values ARDparams.temp and the ARDparams.time are set. The remaining
// four values, ARDparams.pwmNow and ARDparams.HeatCool, ARDparams.error and ARDparams.Integral are determined within 
// the modes Manual, Proportional and P/I. 

    ARDparams.temp = T0;
    ARDparams.T1 = T1;
    ARDparams.T2 = T2;
    ARDparams.T3 = T3;
    ARDparams.T4 = T4;
    ARDparams.T5 = T5;

    float previousTime = ARDparams.time;  // save previous elapsed time before updating new time. Used for dt for (integral) P/I control
    unsigned long currentTime = millis();
    ARDparams.time = (currentTime - startTime) / 1000.0;  // update current elapsed time
    float dt = 0;  // define time interval dt

// Each cycle read in the Matlab parameters, MLparams.
// Parsing strings is a pain. This works, but is ugly.
  if (Serial.available() > 0) {
    receivedString = Serial.readStringUntil('\n'); // Read until a newline character is encountered
    FromMatlab = receivedString;
    // 9 Params from Matlab
    // [Mode PWM H/C Kp KI Tset ResetInt SinAmp SinPer] with formats [%s %d %d %f %f %f %d %f %f] - one blank space between each field. No carriage return!
    // Use a String tokenizer to split the received string at spaces
   
       // Split the received string at spaces using strtok
    char *token = strtok(receivedString.c_str(), " ");
    
    if (token != nullptr) {
        MLparams.mode = String(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.pwmNow = atoi(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.HeatCool = atoi(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.Kp = atof(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.KI = atof(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.Tset = atof(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.ResetInt = atoi(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.SinAmp = atof(token);
        token = strtok(nullptr, " ");
    }

    if (token != nullptr) {
        MLparams.SinPer = atof(token);
    }
   
    }

// ************************* Manual Mode ***************************
// Determine ARDparams.pwmNow and ARDparams.HeatCool for Manual mode.
// Simply copy the values from Matlab.

if (strcmp(MLparams.mode.c_str(), "Manual") == 0) {
  ARDparams.pwmNow = MLparams.pwmNow;
  ARDparams.HeatCool = MLparams.HeatCool;
}

// ************************* Proportional Mode ***************************
// Determine ARDparams.pwmNow and ARDparams.HeatCool for Proportional mode.
// Calculate the error = Tset - temp
// HeatCool = 1 means heat, which should be done when Tset > temp
// HeatCool = -1 means cool, which should be done when temp > Tset

else if (strcmp(MLparams.mode.c_str(), "Proportional") == 0) {
      ARDparams.error = MLparams.Tset - ARDparams.temp;
      ARDparams.pwmNow = round(min(max(MLparams.Kp * abs(ARDparams.error), 0), 255));
      // heat or cool
      ARDparams.HeatCool = sgn(ARDparams.error); 
      } 
 
 //  *************************** Proportional Integral (P/I) control **********************

 else if (strcmp(MLparams.mode.c_str(), "P/I") == 0) {

            dt = ARDparams.time - previousTime; // time interval for previous measurement
            previousTime = ARDparams.time;
           
            ARDparams.error = MLparams.Tset - ARDparams.temp;   // error
            ARDparams.Integral = ARDparams.Integral + dt * ARDparams.error;  // update integral of error
            ARDparams.Integral =  ARDparams.Integral * MLparams.ResetInt;    // zero integral?
            if (MLparams.ResetInt == 0){  // reset zero command
              MLparams.ResetInt = 1;  // allows integration on next iteration
            }
            float u = MLparams.Kp * ARDparams.error + MLparams.KI * ARDparams.Integral;  // calculate PI feedback
            ARDparams.pwmNow = min(abs(round(u)),255);  // magnitude of PI feedback term u limited to a max of 255 for arduino PWM
            ARDparams.HeatCool = sgn(u);  // sign of PI feedback (heat vs cool)
 }

 //  *************************** Tset is sinusoidal with P/I control **********************
 else if (strcmp(MLparams.mode.c_str(), "Sin") == 0) {
   float sinPartTset;

            dt = ARDparams.time - previousTime; // time interval for previous measurement
            previousTime = ARDparams.time;

            // change amplitude of sinusoidal varying Tset when cooling
            // float slopeOfSin = cos(2*PI*ARDparams.time / MLparams.SinPer);
            // if (sgn(slopeOfSin) > 0){
            // sinPartTset =  MLparams.SinAmp * sin(2*PI*ARDparams.time / MLparams.SinPer);
            // }
            // else {
            // float coolToHeatfactor = 2;
            // sinPartTset =  coolToHeatfactor * MLparams.SinAmp * sin(2*PI*ARDparams.time / MLparams.SinPer);
            // }
            sinPartTset =  MLparams.SinAmp * sin(2*PI*ARDparams.time / MLparams.SinPer);
            ARDparams.Tset = MLparams.Tset + sinPartTset;
            ARDparams.error = ARDparams.Tset - ARDparams.temp;   // error
            ARDparams.Integral = ARDparams.Integral + dt * ARDparams.error;  // update integral of error
            ARDparams.Integral =  ARDparams.Integral * MLparams.ResetInt;    // zero integral?
            if (MLparams.ResetInt == 0){  // reset zero command
              MLparams.ResetInt = 1;  // allows integration on next iteration
            }
            float u = MLparams.Kp * ARDparams.error + MLparams.KI * ARDparams.Integral;  // calculate PI feedback
// Increase PWM when cooling. dT/dPWM heating = 2 * dT/dPWM cooling, so increase PWM by 2 when cooling compared to heating.
            if (u < 0){
               u = 1.4*u;
              }
            ARDparams.pwmNow = min(abs(round(u)),255);  // magnitude of PI feedback term u limited to a max of 255 for arduino PWM
            ARDparams.HeatCool = sgn(u);  // sign of PI feedback (heat vs cool)
 }

 //  *************************** Tset is sinusoidal with P/I control **********************
 else if (strcmp(MLparams.mode.c_str(), "TurnOff") == 0) {
    ARDparams.pwmNow = 0;
 }

   // Error in reading input from Matlab   
      else {
        Serial.print("Error reported in Arduino. Received Command: [");
        Serial.print(FromMatlab);
        Serial.print("], Mode: ");
        Serial.print(MLparams.mode);
        Serial.print(", PWM: ");
        Serial.print(MLparams.pwmNow);
        Serial.print(", H/C: ");
        Serial.print(MLparams.HeatCool);
        Serial.print(", Kp: ");
        Serial.print(MLparams.Kp);
        Serial.print(", KI: ");
        Serial.print(MLparams.KI);
        Serial.print(", Tset: ");
        Serial.print(MLparams.Tset);
        Serial.print(", Time = ");
        Serial.print(ARDparams.time, 1); // Print elapsed time with 1 decimal place
        Serial.println(" seconds");
      }

      // Send PWM values to H-bridge

      if (ARDparams.HeatCool == 1){
      analogWrite(pwmOut1, ARDparams.pwmNow);
      analogWrite(pwmOut2,0); 
    }
    if (ARDparams.HeatCool == -1){
      analogWrite(pwmOut2, ARDparams.pwmNow);
      analogWrite(pwmOut1,0); 
    }

if (strcmp(MLparams.mode.c_str(), "Sin") != 0) {
  ARDparams.Tset = MLparams.Tset;
}

// Send the 10 ARDparams (temperature, time, PWM, heat/cool, error, integral, Tset, T1, T2, T3) to Matlab for plotting
        Serial.print("Temperature = ");
        Serial.print(ARDparams.temp);  // Print temperature
        Serial.print(", Time = ");
        Serial.print(ARDparams.time, 1); // Print elapsed time with 1 decimal place
        Serial.print(", PWM = ");
        Serial.print(ARDparams.pwmNow);
        Serial.print(", Heat/Cool = ");
        Serial.print(ARDparams.HeatCool);
        Serial.print(", error = ");
        Serial.print(ARDparams.error);
        Serial.print(", KI * Integral = ");
        Serial.print(MLparams.KI * ARDparams.Integral);
        Serial.print(", Tset = ");
        Serial.print(ARDparams.Tset);
        Serial.print(", T1 = ");
        Serial.print(ARDparams.T1);
        Serial.print(", T2 = ");
        Serial.print(ARDparams.T2);
        Serial.print(", T3 = ");
        Serial.print(ARDparams.T3);
        Serial.print(", T4 = ");
        Serial.print(ARDparams.T4);
        Serial.print(", T5 = ");
        Serial.println(ARDparams.T5);

        Serial.print("Received Command: [");   // this info gets printed on the Matlab command line for diagnostics
        Serial.print(FromMatlab);
        Serial.print("], Mode: ");
        Serial.print(MLparams.mode);
        Serial.print(", PWM: ");
        Serial.print(MLparams.pwmNow);
        Serial.print(", H/C: ");
        Serial.print(MLparams.HeatCool);
        Serial.print(", Kp: ");
        Serial.print(MLparams.Kp);
        Serial.print(", KI: ");
        Serial.print(MLparams.KI);
        Serial.print(", Tset: ");
        Serial.print(MLparams.Tset);
        Serial.print(", ResetInt = ");
        Serial.print(MLparams.ResetInt);
        Serial.print(", SinAmp = ");
        Serial.print(MLparams.SinAmp);        
        Serial.print(", SinPer = ");
        Serial.print(MLparams.SinPer);
        Serial.print(", dt = ");
        Serial.print(dt);
        Serial.print(", Time = ");
        Serial.print(ARDparams.time, 1); // Print elapsed time with 1 decimal place
        Serial.println(" seconds");
  }
  // sign of a number
  static inline int8_t sgn(float val) {
  if (val < 0) return -1;
  return 1;
}
float measureTemperature(int analogPin, int numMeasurements) {
  float total = 0;
  for (int i = 0; i < numMeasurements; i++) {
    int sensorValue = analogRead(analogPin);
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
  
  return temperature;
}
