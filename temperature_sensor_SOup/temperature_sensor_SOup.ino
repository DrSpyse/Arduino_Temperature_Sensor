#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <avr/pgmspace.h>
Adafruit_ADS1115 ads1115;

//Constants for voltage to temperature equation:
//const double A = 340.279;
//const double B = -39.4879;
//const double C = 0.0635671;
//Above constants are old.
//For new equation
const double A = 298.603;
const double B = -17.0318;
const double C = -13.6843;
//ENDS HERE-------------------------------------

int r = 11;
int g = 12;
int b = 13;
int x = 9; // This pin is not connected to anything intentionally

int togglePin = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//HERE IS HOW YOU USE THIS:                                                                                      //
//r = red, g = green, b = blue, x = no LED                                                                       //
//will light up all LEDs in the column at the same index as the nearest range that the temperature is LESS THAN. //
//                                                                                                               //
//For example if the ranges were:                                                                                //
//                                                                                                               //
//{10.0, 20.0, 30.0, 40.0}                                                                                       //
//                                                                                                               //
//and the LEDs were:                                                                                             //
//                                                                                                               //
//{b, b, g, r, r},                                                                                               //
//{x, g, x, g, x},                                                                                               //
//{x, x, x, x, x}                                                                                                //
//                                                                                                               //
//and the temperature was 15.0                                                                                   //
//                                                                                                               //
//since 15.0 is greater than 10.0 and less than 20.0, and 20.0 is the SECOND element in ranges[]                 //
//the BLUE and GREEN LEDs would light up because they are in the SECOND column in LEDs[][]                       //
//                                                                                                               //
//The amount of columns in LEDs should always equal the number of elements in ranges + 1                         //
//As long as the above is true you can add as many or as few ranges as you like                                  //
//                                                                                                               //
//If you have any questions or this doesn't work text me @ 610-662-3339                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ranges[] = {15.0, 30.0};
int LEDs[3][sizeof(ranges) / sizeof(ranges[0]) + 1] = {
                  {b, x, b,},
                  {x, g, x,},
                  {x, x, r,}
    };
//END--------------------------------------------------------------------------------------------------------------
                
const double tuningConstant = 0.0;//change this to add an offset to temperature reading

double temperature = 0.0;

double getTempOLD(double voltage) {
  return (A + B * log(((voltage / 5) * 8850) / (1 - (voltage / 5))) + C * pow(log(((voltage / 5) * 8850) / (1 - (voltage / 5))), 3)) - tuningConstant;
}

//Currently using this
double getTemp(double voltage) {
  return (A + B * log(((voltage / 5) * 8850) / (1 - (voltage / 5))) + C * log((voltage / 5) * 8850)) - tuningConstant;
}

void setup() {

  Serial.begin(57600);
  ads1115.begin(); // Initialize ads1115
  ads1115.setGain(GAIN_ONE);
  Serial.println("Initalized");

  //Sets pin modes
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(x, OUTPUT);

  pinMode(togglePin, OUTPUT);
}

void loop() {

  //Connects thermistor to power via relay with delay to allow the values to be read
  digitalWrite(togglePin, HIGH);
  delay(200);

  int16_t adc0, adc1, adc2, adc3;
  adc0 = ads1115.readADC_SingleEnded(0); 
 
 
  Serial.print("Voltage: ");
  float val = adc0 * .000125; 
  Serial.println(val, 21);

  temperature = getTemp(val);

  //Calls method
  lightLED();
  
  Serial.print("Temperature(C): ");
  Serial.println(temperature);

  int sensorValue = analogRead(A0);
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  //Disconnects thermistor to power via relay
  digitalWrite(togglePin, LOW);
  
  Serial.println("----------------------"); //This is just a divider to organize the serial monitor.
  
  delay(1000);
}

void lightLED(){
  //Resets pins and variables
  bool breakLoop = false;
  digitalWrite(r, LOW);
  digitalWrite(g, LOW);
  digitalWrite(b, LOW);
  digitalWrite(x, LOW);

  //DEBUGGING
  //Serial.print("Max Range: ");
  //Serial.println(ranges[sizeof(ranges) / sizeof(ranges[0]) - 1]);
  
  //If the temperature is greater than the last value in the ranges array
  if(temperature > ranges[sizeof(ranges) / sizeof(ranges[0]) - 1]){
    //lights up the LEDs in the last column of the LEDs array
    for (int y = 0; y < 3; y++){
        digitalWrite(LEDs[y][sizeof(ranges) / sizeof(ranges[0])], HIGH);
    }
  } else {
    //goes through the ranges array and accesses its values
    for(int x = 0; x < sizeof(ranges) / sizeof(ranges[0] - 1); x++){
      //If the temperature is less than the range in ranges at index x
      if(temperature <= ranges[x]){
        //light up corresponding LEDs
        for (int y = 0; y < 3; y++){
          digitalWrite(LEDs[y][x], HIGH);
          breakLoop = true;
        }
      }
      if(breakLoop){
        break;
      }
    }
  }
}
