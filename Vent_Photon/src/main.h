/*
 * Project Vent_Photon
  * Description:
  * Combine digital pot output in parallel with manual pot
  * to control an ECMF-150 TerraBloom brushless DC servomotor fan.
  * 
  * By:  Dave Gutz January 2021
  * 07-Jan-2021   Tinker version
  * 
//
// MIT License
//
// Copyright (C) 2021 - Dave Gutz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

* Found MCP4151 POT how to at
  https://community.particle.io/t/photon-controlling-5v-output-using-mcp4151-pot-and-photon-spi-api/25001/2

* There is a POT library here.   Haven't used it yet
  https://github.com/jmalloc/arduino-mcp4xxx

****Note about ground
* For ICs to work, I believe ECMF B (ground) needs to be connected to
  Photon and IC ground.  Need to try this.  Will have to find an isolation scheme if this doesn't work,
  using ECMF 10V and it's ground for one supply and 5v and Photon for other.

* ECMF-150 remote brushless DC-motor fan. 300 mA max with auto protect.  (Photon is 1000 mA max)
  R Red wire 10V supply generated by ECMF to PWM Driver Circuit 210 ohm resistor-H
  B Black wire GND.   ????Don't know if this is earth.  I guess it floats.  Try tie to Photon
  C Blue wire 0-10V control signal used by ECMF to modulate fan speed 0-100%.   Can either be  
        1-10kHz PWM or 10K pot wiper connected H to R and L to B
  Y Tach signal 0-10V for 0-100%. Isink_max 10mA.  I don't know what that means.

* Pot Analog Connections
  POHa    ECMF R 10 V supply
  POWa    Photon A2 (BOM = ECMF C Blue Control Signal)
  POLa    ???System GND to GND Rail

article Photon boards have 9 PWM pins: D0, D1, D2, D3, A4, A5, WKP, RX, TX

* PWM Driver Circuit
  - npn1 (2n2222A)
    C - B of npn2 and L of 4k7 that goes to 10V
    B - H of 10k
    E - GND Rail
  - npn2 (2n2222A)
    C - ECMF C and diode anode?? and pullup that goes to 10V
    B - C of npn1 and 4k7 that goes to 10V
    E - GND Rail
  - pullup 4k7
    H - ECMF 10V
    L - C of npn2 and H of diode
  - 10k1 npn1 base driver
    H - Photon D2
    L - B of npn1
  - 10k2 npn2 base driver
    H - C of npn1
    L - B of npn2
  - diode (1N4148) signal diode to protect npn2 from load transients
    Anode?? - ECMF C
    Cathode?? - GND Rail

* Tach Voltage Divider Circuit
  - 200K ohm resistor
    H - ECMF Y - Tach
    L - Photon A8
  - 100K ohm resistor
    H - Photon A1
    L - Ground Rail

* Honeywell temp/humidity Hardware Connections (Humidistat with temp SOIC  HIH6131-021-001)
  Wire.h and I2C used.   
  Code originally developed for gitHub davegutz/myThermostat-Particle-Photon/myThermostat_Particle_DEV/myThermostat.ino.
  1-VCORE= 0.1uF jumper to GND
  2-VSS  = GND Rail
  3-SCL  = D1
  4-SCA  = D0
  5-AL_H = NC
  6-AL_L = NC
  7-NC   = NC
  8-VDD  = 3v3

* Particle Photon 1A max
  GND = to 2 GND rails
  A1  = L of 200k ohm from ECMF Y Tach and H of 100k ohm to ground (0-3.3v from 0-10v)
  A2  = POWa of analog POT
  D0  = 4-SCA of Honeywell and 4k7 3v3 jumper I2C pullup
  D1  = 3-SCL of Honeywell and 4k7 3v3 jumper I2C pullup
  D2  = H of 10k1 for PWM 5kHz
  D6  = Y-C of DS18 for Tp and 4k7 3v3 jumper pullup
  VIN = 5V Rail 1A maximum and 0.1uF to GND and 100uF to GND
  3v3 = 3v3 rail out
  micro USB = Serial Monitor on PC (either Particle Workbench monitor or CoolTerm) 

* Voltage spike protection spec'd by Particle
  - 0.1uF
    H - VIN and 5V Rail
    L - GND Rail
  - 100uF
    H - VIN and 5V Rail
    L - GND Rail

* 1-wire Temp (MAXIM DS18B20)  library at https://github.com/particle-iot/OneWireLibrary
  Y-C   = Photon D6
  R-VDD = 5V Rail
  B-GND = GND Rail

* 5v
  5v to 5V Rail.  From wall wart in attic. 1 A max sized for Photon wifi transients
  Gnd to GND Rail.   from wall wart in attic.  1 A max sized for Photon wifi transients

* *****not used Elego power module mounted to 5V and 3v3 and GND rails
  5V jumper = 5V RAIL on "A-side" of Photon
  Jumper "D-side" of Photon set to OFF
  Round power supply = round power supply plug 12 VDC x 1.0A Csec CS12b20100FUF
  
 * Author: Dave Gutz davegutz@alum.mit.edu  repository GITHUB myVentilator
 
  To get debug data
  1.  Set debug = 2 in constants.h
  2.  Rebuild and upload
  3.  Start CoolTerm_0.stc

  Requirements:
  1.  Wire digital POT in parallel with supplied 10K hardware POT.
  2.  When Elego power off, digital POT off and digital POT resistance = open circuit

*/

// For Photon
#if (PLATFORM_ID == 6)
#define PHOTON
#include "application.h" // Should not be needed if file ino or Arduino
SYSTEM_THREAD(ENABLED); // Make sure code always run regardless of network status
#include <Arduino.h>     // Used instead of Print.h - breaks Serial
#else
#undef PHOTON
using namespace std;
#undef max
#undef min
#endif

#include "constants.h"
#include <OneWire.h>
#include <DS18.h>
														   
// Global locals
char buffer[256];           // Serial print buffer
int hum = 0;                // Relative humidity integer value, %
int I2C_Status = 0;         // Bus status
double Ta_Sense = NOMSET;   // Sensed ambient room temp, F
double Tp_Sense = NOMSET;   // Sensed plenum temp, F
double updateTime = 0.0;    // Control law update time, sec
int numTimeouts = 0;        // Number of Particle.connect() needed to unfreeze
bool webHold = false;       // Web permanence request
int webDmd = 62;            // Web sched, F
double pcnt_pot = 0;        // Potentiometer read, % of 0-10v
double pcnt_tach = 0;       // Tach read, % of 0-10v
uint32_t duty = 0;          // PWM duty cycle, 255-0 counts for 0-100% on ECMF-C

#ifdef PHOTON
byte pin_1_wire = D6;       // 1-wire Plenum temperature sensor
pin_t pwm_pin = D2;         // Power the PWM transistor base via 300k resistor
byte status_led = D7;       // On-board led
byte tach_sense = A1;       // Sense ECMF speed
byte pot_sense = A2;        // Sense Pot
#endif

// Utilities
void serial_print_inputs(unsigned long now, double run_time, double T);
void serial_print(uint32_t duty);
uint32_t pwm_write(uint32_t duty);
boolean load(int reset, double T, unsigned int time_ms);
DS18 sensor_plenum(pin_1_wire);

#ifndef NO_CLOUD
int particleHold(String command)
{
  if (command == "HOLD")
  {
    webHold = true;
    return 1;
  }
  else
  {
     webHold = false;
     return 0;
  }
}


int particleSet(String command)
{
  int possibleSet = atoi(command);
  if (possibleSet >= MINSET && possibleSet <= MAXSET)
  {
      webDmd = possibleSet;
      return possibleSet;
  }
  else return -1;
}
#endif


// Setup
void setup()
{

#ifdef PHOTON
  // WiFi.disconnect();
  // delay(1000);
  // WiFi.off();
  // delay(1000);
#endif

  // Serial
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  Serial.flush();
  delay(1000);  // Ensures a clean display on Arduino Serial startup on CoolTerm

  // Peripherals
  if ( !bare )
  {
    // Status
    pinMode(status_led, OUTPUT);
    digitalWrite(status_led, LOW);

    // PWM Control
    pinMode(pwm_pin, OUTPUT);

    // I2C
    Wire.setSpeed(CLOCK_SPEED_100KHZ);
    Wire.begin();

    // Initialize output (255 = off)
    pwm_write(duty);
  }

  // Begin
  Particle.connect();
  #ifndef NO_CLOUD
    Particle.function("HOLD", particleHold);
    Particle.function("SET",  particleSet);
  #endif

  

#ifdef PHOTON
  if ( debug>1 ) { sprintf(buffer, "Particle Photon.  bare = %d,\n", bare); Serial.print(buffer); };
#else
  if ( debug>1 ) { sprintf(buffer, "Arduino Mega2560.  bare = %d,\n", bare); Serial.print(buffer); };
#endif

  // Header for debug print
  if ( debug>1 )
  { 
    Serial.print(F("flag,time_ms,run_time,T,I2C_Status,Tp_Sense,Ta_Sense,hum,pot,tach,duty,")); Serial.println("");
  }

  if ( debug>3 ) { Serial.print(F("End setup debug message=")); Serial.println(F(", "));};

} // setup


// Loop
void loop()
{
  static unsigned long now = millis();      // Keep track of time
  static unsigned long past = millis();     // Keep track of time
  static boolean toggle = false;            // Generate heartbeat
  static double run_time = 0;               // Time, seconds
  static int reset = 1;                     // Dynamic reset
  // static boolean was_testing = true;        // Memory of testing, used to perform a logic reset on transition
  double T = 0;                             // Present update time, s
  boolean testing = true;                   // Initial startup is calibration mode to 60 bpm, 99% spo2, 2% PI
  const int bare_wait = int(1000.0);        // To simulate peripherals sample time
  static double cmd = 0;                  // PWM duty cycle output
  bool control;            // Control sequence, T/F
  bool display;            // LED display sequence, T/F
  bool filter;             // Filter for temperature, T/F
  bool model;              // Run model, T/F
  bool publishAny;         // Publish, T/F
  bool publish1;           // Publish, T/F
  bool publish2;           // Publish, T/F
  bool publish3;           // Publish, T/F
  bool publish4;           // Publish, T/F
  bool query;              // Query schedule and OAT, T/F
  bool read;               // Read, T/F
  static unsigned long    lastControl  = 0UL; // Last control law time, ms
  static unsigned long    lastDisplay  = 0UL; // Las display time, ms
  static unsigned long    lastFilter   = 0UL; // Last filter time, ms
  static unsigned long    lastModel    = 0UL; // Las model time, ms
  static unsigned long    lastPublish1 = 0UL; // Last publish time, ms
  static unsigned long    lastPublish2 = 0UL; // Last publish time, ms
  static unsigned long    lastPublish3 = 0UL; // Last publish time, ms
  static unsigned long    lastPublish4 = 0UL; // Last publish time, ms
  static unsigned long    lastQuery    = 0UL; // Last read time, ms
  static unsigned long    lastRead     = 0UL; // Last read time, ms
  static double           tFilter;            // Modeled temp, F

  // Sequencing
  filter = ((now-lastFilter)>=FILTER_DELAY) || reset>0;
  if ( filter )
  {
    tFilter     = float(now-lastFilter)/1000.0;
    if ( debug > 3 ) Serial.printf("Filter update=%7.3f\n", tFilter);
    lastFilter    = now;
  }

  model     = ((now-lastModel)>=MODEL_DELAY) || reset>0;
  if ( model )
  {
    if ( debug > 3 ) Serial.printf("Model update=%7.3f\n", float(now-lastModel)/1000.0);
    lastModel    = now;
  }

  publish1  = ((now-lastPublish1) >= PUBLISH_DELAY*4);
  if ( publish1 ) lastPublish1  = now;
  publish2  = ((now-lastPublish2) >= PUBLISH_DELAY*4)  && ((now-lastPublish1) >= PUBLISH_DELAY);
  if ( publish2 ) lastPublish2  = now;
  publish3  = ((now-lastPublish3) >= PUBLISH_DELAY*4)  && ((now-lastPublish1) >= PUBLISH_DELAY*2);
  if ( publish3 ) lastPublish3  = now;
  publish4  = ((now-lastPublish4) >= PUBLISH_DELAY*4)  && ((now-lastPublish1) >= PUBLISH_DELAY*3);
  if ( publish4 ) lastPublish4  = now;
  publishAny  = publish1 || publish2 || publish3 || publish4;

  read    = ((now-lastRead) >= READ_DELAY || reset>0) && !publishAny;
  if ( read     ) lastRead      = now;

  query   = ((now-lastQuery)>= QUERY_DELAY) && !read;
  if ( query    ) lastQuery     = now;

  display   = ((now-lastDisplay) >= DISPLAY_DELAY) && !query;
  if ( display ) lastDisplay    = now;


  // Sample inputs
  past = now;
  now = millis();
  T = (now - past)/1e3;
  unsigned long deltaT = now - lastControl;
//  control = (deltaT>=CONTROL_DELAY) && !display;
  control = (deltaT>=CONTROL_DELAY) || reset;
  if ( control  )
  {
    updateTime    = float(deltaT)/1000.0 + float(numTimeouts)/100.0;
    lastControl   = now;
  }

  delay(500);

  if ( bare )
  {
    delay ( bare_wait );
  }
  run_time += T;
  if ( debug>3 ) { Serial.print(F("debug loop message here=")); Serial.println(F(", ")); };

  // Outputs
  if ( control )
  {
    cmd = pcnt_pot;
    duty = uint32_t(cmd*256.0/100.0);
    pwm_write(duty);
    toggle = !toggle;
    digitalWrite(status_led, HIGH);
  }

  // Read sensors
  if ( read )
  {
    if ( !bare )
    {
      if ( debug>3 ) Serial.println(F("read"));
      testing = load(reset, T, now);
      testing = testing;

    }
    else
    {
      delay(41); // Usual I2C time
      if ( reset>0 ) Ta_Sense = NOMSET;
      if ( reset>0 ) Tp_Sense = NOMSET;
    }
  }


  // Publish
  if ( publishAny && debug>3 )
  {
    if ( debug>3 ) Serial.println(F("publish"));
  }

  // Monitor
  if ( debug>1 && display )
  {
    serial_print_inputs(now, run_time, T);
    serial_print(duty);
  }

  // Initialize complete
  reset = 0;

} // loop


// Inputs serial print
void serial_print_inputs(unsigned long now, double run_time, double T)
{
  Serial.print(F("0,")); Serial.print(now, DEC); Serial.print(", ");
  Serial.print(run_time, 3); Serial.print(", ");
  Serial.print(T, 6); Serial.print(", ");  
  Serial.print(I2C_Status, DEC); Serial.print(", ");
  Serial.print(Tp_Sense, 1); Serial.print(", ");
  Serial.print(Ta_Sense, 1); Serial.print(", ");
  Serial.print(hum, 1); Serial.print(", ");
  Serial.print(pcnt_pot, 1); Serial.print(", ");
  Serial.print(pcnt_tach, 1); Serial.print(", ");
}

// Normal serial print
void serial_print(uint32_t duty)
{
  if ( debug>0 )
  {
    Serial.print(duty, DEC); Serial.print(F(", "));   Serial.println(F(""));
  }
  else
  {
  }
  
}

// Load and filter
// TODO:   move 'read' stuff here
boolean load(int reset, double T, unsigned int time_ms)
{
  static boolean done_testing = false;

  // Read Sensor
  if ( !bare )
  {
    // Honeywell Read
    Wire.beginTransmission(TA_SENSOR);
    Wire.endTransmission();
    delay(40);
    Wire.requestFrom(TA_SENSOR, 4);
    Wire.write(byte(0));
    uint8_t b = Wire.read();
    I2C_Status = b >> 6;

    // Honeywell conversion
    int rawHum  = (b << 8) & 0x3f00;
    rawHum |=Wire.read();
    hum = roundf(rawHum / 163.83) + HW_HUMCAL;
    int rawTemp = (Wire.read() << 6) & 0x3fc0;
    rawTemp |=Wire.read() >> 2;
    Ta_Sense = (float(rawTemp)*165.0/16383.0 - 40.0)*1.8 + 32.0 + TA_TEMPCAL; // convert to fahrenheit and calibrate

    // MAXIM conversion 1-wire Tp plenum temperature
    if (sensor_plenum.read()) Tp_Sense = sensor_plenum.fahrenheit() + TP_TEMPCAL;

    // Pot input
    int raw_pot = analogRead(pot_sense);
    pcnt_pot = double(raw_pot)/40.96;

    // Tach input
    int raw_tach = analogRead(tach_sense);
    pcnt_tach = double(raw_tach)/40.96;
  }
  else
  {
  }

  // Built-in-test logic.   Run until finger detected
  if ( true && !done_testing )
  {
    done_testing = true;
  }
  else                    // Possible finger detected
  {
    done_testing = false;
  }

  // Built-in-test signal replaces sensor
  if ( !done_testing )
  {
  }

  return ( !done_testing );
}

uint32_t pwm_write(uint32_t duty)
{
    analogWrite(pwm_pin, duty, pwm_frequency);
    return duty;
}
