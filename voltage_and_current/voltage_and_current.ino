// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#define LED 9
#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance

void setup()
{  
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  
  emon1.voltage(3, 226.92 , 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(0, 111.1);       // Current: input pin, calibration.
}

void loop()
{
  emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
  emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
  float realPower       = emon1.realPower;        //extract Real Power into variable
  float apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
  float powerFActor     = emon1.powerFactor;      //extract Power Factor into Variable
  float supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
  float Irms            = emon1.Irms;             //extract Irms into Variable
  
  digitalWrite(LED,!digitalRead(LED));
  delay(5000);
  
}
