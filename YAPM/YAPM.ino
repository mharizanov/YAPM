/*
 Yet Another Power Monitor Project (YAPM)
 Power monitor based on the OpenEnergyMonitor  EmonTx CT123 + Voltage example"
 
 http://harizanov.com/2013/11/the-yapm-pcb-is-done/ 
 
 An example sketch for the emontx module for
 CT and AC voltage sample electricity monitoring. Enables real power and Vrms calculations.
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 
 emonTx documentation: http://openenergymonitor.org/emon/modules/emontxshield/
 emonTx firmware code explination: http://openenergymonitor.org/emon/modules/emontx/firmware
 emonTx calibration instructions: http://openenergymonitor.org/emon/modules/emontx/firmware/calibration

 THIS SKETCH REQUIRES:

 Libraries in the standard arduino libraries folder:
	- JeeLib		https://github.com/jcw/jeelib
	- EmonLib		https://github.com/openenergymonitor/EmonLib.git

 Other files in project directory (should appear in the arduino tabs above)
	- emontx_lib.ino

*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
-ID-	-Node Type- 
0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
1-4     - Control nodes 
5-10	- Energy monitoring nodes
11-14	--Un-assigned --
15-16	- Base Station & logging nodes
17-30	- Environmental sensing nodes (temperature humidity etc.)
31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
-------------------------------------------------------------------------------------------------------------
*/


#include <JeeLib.h>
#define FILTERSETTLETIME 5000                                          //  Time (ms) to allow the filters to settle before sending data
//CT 1 is always enabled

#define freq RF12_868MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 28;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                    // the UNO bootloader                                                    // the UNO bootloader 
                                                                                                    // Download JeeLib: http://github.com/jcw/jeelib

#include "EmonLib.h"
EnergyMonitor ct1;                                              // Create  instances for each CT channel

typedef struct { int power1, Vrms; } PayloadTX;         // neat way of packaging data for RF comms
PayloadTX emontx;

const int LEDpin = 9;                                                   // On-board emonTx LED 

boolean settled = false;

void setup() 
{
  Serial.begin(9600);
  Serial.println("YAPM - Yet Another Power Monitor");
  Serial.println("harizanov.com");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
  
  ct1.voltage(3,226.92, 1.7);                                       // ct.voltageTX(calibration, phase_shift) - make sure to select correct calibration for AC-AC adapter  http://openenergymonitor.org/emon/modules/emontx/firmware/calibration. Default is set for Ideal Power voltage adapter. 
  ct1.current(0, 111.1);                                            // Setup emonTX CT channel (channel (1,2 or 3), calibration)
                                                                      // CT Calibration factor = CT ratio / burden resistance  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RF
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S);                                         // Enable anti crash (restart) watchdog if UNO bootloader is selected. Watchdog does not work with duemilanove bootloader                                                             // Restarts emonTx if sketch hangs for more than 8s
}

void loop() 
{ 
  ct1.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
  emontx.power1 = ct1.realPower;
  Serial.print(emontx.power1); 
  emontx.Vrms = ct1.Vrms*100;                                          // AC Mains rms voltage 
  Serial.print(" "); Serial.print(ct1.Vrms);
  Serial.println(); delay(100);

  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
    do_sleep(20);      // sleep or delay in seconds - see emontx_lib
  }
}

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);
  // if ready to send + exit loop if it gets stuck as it seems too
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
  rf12_sendStart(0, &emontx, sizeof emontx);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}


void do_sleep(int seconds) {
  // if (emontx.battery > 3300) { 
    for (int i=0; i<seconds; i++) { 
      delay(1000); 
      if (UNO) wdt_reset();
    } 
  // } else Sleepy::loseSomeTime(seconds*1000);
}
