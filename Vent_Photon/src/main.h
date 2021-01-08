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

* Hardware Connections (MCP4151-103, 10k nom, to Photon):
  -1-CS   = D5
  -2-SCK  = D4
  -3-MOSI = D2  (4k7 to D3 jumper)
  -4-GND  = GND RAIL
  -5-POA  = 
  -6-POW  =
  -7-POB  =
  -8-VDD  = D6 and pullup to 5V RAIL using 4.7k

* Photon to Proto
  GND to 2 GND RAILS
  D2 4k7 D3 jumper
  VIN to 5V
  micro USB to Serial Monitor on PC (either Particle Workbench monitor or CoolTerm) 

* Elego power module
  5V jumper to 5V RAIL on "A-side" of Photon
  Jumper on "D-side" of Photon set to OFF
  Round power supply plug 12 VDC x 1.0A Csec CS12b20100FUF
  
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
															   
char buffer[256];           // Serial print buffer

// Utilities
void serial_print_inputs(unsigned long now, double T);
void serial_print(int i);
int pot_write(int step);
boolean load(int reset, double T, unsigned int time_us);

#ifdef PHOTON
byte led_pulse = D7; //Blinks with each heartbeat
byte vdd_supply = D6;  // Power the MCP4151
#else
byte led_pulse = 13; //Blinks with each heartbeat
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

  // MCP4151 using SPI1 and Digtial output 
  SPI1.setBitOrder(MSBFIRST);
  SPI1.setClockSpeed(10,MHZ);
  SPI1.setDataMode(SPI_MODE0);
  SPI1.begin();

  // Serial
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  Serial.flush();
  delay(1000);  // Ensures a clean display on Arduino Serial startup on CoolTerm

#ifdef PHOTON
  if ( debug>1 ) { sprintf(buffer, "Particle Photon.  bare = %d,\n", bare); Serial.print(buffer); };
#else
  if ( debug>1 ) { sprintf(buffer, "Arduino Mega2560.  bare = %d,\n", bare); Serial.print(buffer); };
#endif

  // LEDs
  pinMode(led_pulse, OUTPUT);


  // Peripherals
  if ( !bare )
  {
    // MCP4151 power supply
    pinMode(vdd_supply, OUTPUT);
  }

  if ( debug>3 ) { Serial.print(F("End setup debug message=")); Serial.println(F(", "));};

} // setup


// Loop
void loop()
{
  static unsigned long now = micros();      // Keep track of time
  static unsigned long past = micros();     // Keep track of time
  static boolean toggle = false;            // Generate heartbeat
  static double run_time = 0;               // Time, seconds
  static int reset = 1;                     // Dynamic reset
  // static boolean was_testing = true;        // Memory of testing, used to perform a logic reset on transition
  double T = 0;                             // Present update time, s
  boolean testing = true;                   // Initial startup is calibration mode to 60 bpm, 99% spo2, 2% PI
  const int bare_wait = int(1000.0);        // To simulate peripherals sample time
  static int cmd = 240;

  // Sample inputs
  past = now;
  now = micros();
  T = (now - past)/1e6;
  // was_testing = testing;
  testing = load(reset, T, now);
  testing = testing;
  digitalWrite(vdd_supply, HIGH);
  delay(2000);

  if ( bare )
  {
    delay ( bare_wait );
  }
  run_time += T;

  // Initialize
  reset = 0;
  if ( debug>3 ) { Serial.print(F("debug loop message here=")); Serial.println(F(", ")); };

  // Outputs
  if (debug>1)
  {
    serial_print_inputs(now, T);
    serial_print(cmd);
  }
  pot_write(cmd);
  digitalWrite(led_pulse, toggle);
  toggle = !toggle;
  delay(2000);

  cmd += 1;
  if (cmd>256) cmd=240;

} // loop


// Inputs serial print
void serial_print_inputs(unsigned long now, double T)
{
  Serial.print(F("0,")); Serial.print(now, DEC); Serial.print(",");
  Serial.print(T, 6); Serial.print(",");  
}

// Normal serial print
void serial_print(int i)
{
  if (debug > 0)
  {
    Serial.print(i, DEC); Serial.print(F(", "));   Serial.println(F(""));
  }
  else
  {
  }
  
}

// Load and filter
boolean load(int reset, double T, unsigned int time_us)
{
  static boolean done_testing = false;

  // Read Sensor
  if ( !bare )
  {
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

int pot_write(int step)
{
    digitalWrite(D5, LOW);
    SPI1.transfer(0);
    SPI1.transfer(step);
    digitalWrite(D5, HIGH);
    return step;
}