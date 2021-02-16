/*
 * Project Vent_Photon
  * Description:
  * Combine digital pot output in parallel with manual pot
  * to control an ECMF-150 TerraBloom brushless DC servomotor fan.
  * 
  * By:  Dave Gutz January 2021
  * 07-Jan-2021   A tinker version
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

****digipot not used
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
  POLa    System GND to GND Rail

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
* Particle Photon boards have 9 PWM pins: D0, D1, D2, D3, A4, A5, WKP, RX, TX
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
  1.  

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
#include "myAuth.h"
#include "myFilters.h"
#include "myRoom.h"
/* This file myAuth.h is not in Git repository because it contains personal information.
Make it yourself.   It should look like this, with your personal authorizations:
(Note:  you don't need a valid number for one of the blynkAuth if not using it.)
#ifndef BARE_PHOTON
  const   String      blynkAuth     = "4f1de4949e4c4020882efd3e61XXX6cd"; // Photon thermostat
#else
  const   String      blynkAuth     = "d2140898f7b94373a78XXX158a3403a1"; // Bare photon
#endif
*/
// Dependent includes.   Easier to debug code if remove unused include files
#ifndef NO_BLYNK
  #include "blynk.h"
  #include "Blynk/BlynkHandlers.h"
#endif
#include <OneWire.h>
#include <DS18.h>
#include "myInsolation.h"

// Global locals
char buffer[256];           // Serial print buffer
int hum = 68;               // Relative humidity integer value, %
int I2C_Status = 0;         // Bus status
double Ta_Sense = NOMSET;   // Sensed ambient room temp, F
double Ta_Filt = NOMSET;    // Filtered, sensed ambient room temp, F
double Tp_Sense = NOMSET;   // Sensed plenum temp, F
double updateTime = 0.0;    // Control law update time, sec
int numTimeouts = 0;        // Number of Particle.connect() needed to unfreeze
bool webHold = false;       // Web permanence request
int potValue = 65;          // Dial raw value, F
double webDmd = 65.0;       // Web sched, F
double pcnt_pot = 0;        // Potentiometer read, % of 0-10v
double pcnt_tach = 0;       // Tach read, % of 0-10v
uint32_t duty = 0;          // PWM duty cycle, 255-0 counts for 0-100% on ECMF-C
// Global locals for Blynk TODO:  simplify
String hmString = "00:00";  // time, hh:mm
bool call = false;          // Heat demand to relay control
bool held = false;          // doomed parameter
double callCount;           // Floating point of bool call for calculation
double set = 65.0;          // Selected sched, F
double tempComp;            // Sensed compensated temp, F
int potDmd = 0;             // Pot value, deg F
int schdDmd = 62;           // Sched raw value, F
double OAT = 30;            // Outside air temperature, F
double Ta_Obs = 0;          // Modeled air temp, F
double rejectHeat = 0.0;    // Adjustment to embedded  model to match sensor, F/sec
#ifndef NO_PARTICLE
  String statStr("WAIT..."); // Status string
#endif
char publishString[40];     // For uptime recording
double controlTime = 0.0;   // Decimal time, seconds since 1/1/2021
bool reco;                  // Indicator of recovering on cold days by shifting schedule
enum Mode {POT, WEB, AUTO, SCHD}; // To keep track of mode
Mode controlMode = POT;     // Present control mode
const int EEPROM_ADDR = 1;  // Flash address
#ifndef NO_WEATHER_HOOK
  int badWeatherCall  = 0;  // webhook lookup counter
  long updateweatherhour;   // Last hour weather updated
  bool weatherGood;         // webhook OAT lookup successful, T/F
#endif
double tempf;               // webhook OAT, deg F
double integ = 0;           // Control integrator output, %
double G = 0.150;           // Control gain, r/s = %/F (0.030)
double tau = 600;           // Control lead, s  (600)
double DB = 0.1;            // Half deadband width, deg F (0.5)
double prop = 0;            // Control proportional output, %
double cont = 0;            // Total control output, %
double err = 0;             // Control error, F
bool lastHold = false;      // Web toggled permanent and acknowledged
unsigned long lastSync = millis();// Sync time occassionally.   Recommended by Particle.
double lastChangedWebDmd = webDmd;
double cmd = 0;                  // PWM duty cycle output
double solar_heat = 0;      // Sun heat on sunshine room wall, Btu/hr

DuctTherm* duct;            // Duct model
DuctTherm* ductEmbMod;      // Duct embedded model
RoomTherm* room;            // Room model
RoomTherm* roomEmbMod;      // Room embedded model
General2_Pole* TaSenseFilt; // Sensor noise and general loop filter
Insolation* sun_wall;       // Solar insolation effects

#ifdef PHOTON
byte pin_1_wire = D6;       // 1-wire Plenum temperature sensor
pin_t pwm_pin = D2;         // Power the PWM transistor base via 300k resistor
byte status_led = D7;       // On-board led
byte tach_sense = A1;       // Sense ECMF speed
byte pot_trim = A2;         // Trim Pot
byte pot_control = A3;      // Control Pot
#endif

// Utilities
void serial_print_inputs(unsigned long now, double T);
void serial_print(double cmd);
uint32_t pwm_write(uint32_t duty);
boolean load(int reset, double T);
DS18 sensor_plenum(pin_1_wire);
void publish1(void); void publish2(void); void publish3(void); void publish4(void);
void publish_particle(unsigned long now, bool publishP, double cmd);
int particleHold(String command);
int particleSet(String command);
int setSaveDisplayTemp(double t);
void saveTemperature(const int set, const int webDmd, const int held, const int addr);
void gotWeatherData(const char *name, const char *data);
void getWeather(void);
String tryExtractString(String str, const char* start, const char* end);
double  decimalTime(unsigned long *currentTime, char* tempStr);
BlynkTimer blynk_timer_1, blynk_timer_2, blynk_timer_3, blynk_timer_4;     // Time Blynk events

// Setup
void setup()
{

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

    // Initialize schedule
    setSaveDisplayTemp(0.0);  // assure user reset happened

    // I2C
    if ( !bare )
    {
      Wire.setSpeed(CLOCK_SPEED_100KHZ);
      Wire.begin();
    }
    // Initialize output (255 = off)
    pwm_write(duty);
  }
  else
  {
    // Status
    pinMode(status_led, OUTPUT);
    digitalWrite(status_led, LOW);
  }

  // OAT
  // Lets listen for the hook response
  #ifndef NO_WEATHER_HOOK
    Particle.subscribe("hook-response/get_weather", gotWeatherData, MY_DEVICES);
  #endif

  // Filters
  duct = new DuctTherm("duct", M_AP_0, M_AP_1, M_AP_2, M_AQ_0, M_AQ_1, M_AQ_2, M_DUCT_DIA,
    M_DUCT_TEMP_DROP, M_MDOTL_DECR, M_MDOTL_INCR, M_MUA, M_RHOA);
  room = new RoomTherm("room", M_CPA, M_DN_TADOT, M_DN_TWDOT, M_QCON, M_QLK, M_RSA, M_RSAI,
    M_RSAO, M_TRANS_CONV_LOW, M_TRANS_CONV_HIGH); 
  TaSenseFilt = new General2_Pole(double(READ_DELAY)/1000., 0.05, 0.80, 0.0, 120.);
  sun_wall = new Insolation(SUN_WALL_AREA, SUN_WALL_REFLECTIVITY, GMT);

  // Begin
  Particle.connect();
  #ifndef NO_CLOUD
    Particle.function("HOLD", particleHold);
    Particle.function("SET",  particleSet);
  #endif

  #ifndef NO_BLYNK
    blynk_timer_1.setInterval(PUBLISH_DELAY, publish1);
    blynk_timer_2.setTimeout(1*PUBLISH_DELAY/4, [](){blynk_timer_2.setInterval(PUBLISH_DELAY, publish2);});
    blynk_timer_3.setTimeout(2*PUBLISH_DELAY/4, [](){blynk_timer_3.setInterval(PUBLISH_DELAY, publish3);});
    blynk_timer_4.setTimeout(3*PUBLISH_DELAY/4, [](){blynk_timer_4.setInterval(PUBLISH_DELAY, publish4);});
    Blynk.begin(blynkAuth.c_str());
  #endif

  #ifdef PHOTON
    if ( debug>1 ) { sprintf(buffer, "Particle Photon.  bare = %d,\n", bare); Serial.print(buffer); };
  #else
    if ( debug>1 ) { sprintf(buffer, "Arduino Mega2560.  bare = %d,\n", bare); Serial.print(buffer); };
  #endif

  // Header for debug print
  if ( debug>1 )
  { 
    Serial.print(F("flag,time_ms,controlTime,T,I2C_Status,set,Tp_Sense,Ta_Sense,Ta_Filt,hum,pot,OAT,solar,cmd,duty,")); Serial.println("");
  }

  if ( debug>3 ) { Serial.print(F("End setup debug message=")); Serial.println(F(", "));};

} // setup


// Loop
void loop()
{
  unsigned long currentTime;                // Time result
  static unsigned long now = millis();      // Keep track of time
  static unsigned long past = millis();     // Keep track of time
  static boolean toggle = false;            // Generate heartbeat
  static int reset = 1;                     // Dynamic reset
  // static boolean was_testing = true;        // Memory of testing, used to perform a logic reset on transition
  double T = 0;                             // Present update time, s
  boolean testing = true;                   // Initial startup is calibration mode to 60 bpm, 99% spo2, 2% PI
  const int bare_wait = int(1);        // To simulate peripherals sample time
  bool readTp;             // Special sequence to read Tp affected by PWM noise with duty>0, T/F
  static bool dwellTp;     // Special hold to read Tp T/F
  bool control;            // Control sequence, T/F
  bool display;            // LED display sequence, T/F
  bool publishP;           // Particle publish, T/F
  bool query;              // Query schedule and OAT, T/F
  bool read;               // Read, T/F
  bool serial;             // Serial print, T/F
  bool checkPot = false;   // Read the POT, T/F
  static unsigned long lastReadTp   = 0UL; // Last readTp time, ms
  static unsigned long lastDwellTp  = 0UL; // Last dwellTp time, ms
  static unsigned long lastControl  = 0UL; // Last control law time, ms
  static unsigned long lastDisplay  = 0UL; // Las display time, ms
  static unsigned long lastPublishP = 0UL; // Last publish time, ms
  static unsigned long lastQuery    = 0UL; // Last read time, ms
  static unsigned long lastRead     = 0UL; // Last read time, ms
  static unsigned long lastSerial   = 0UL; // Last Serial print time, ms
  static double last_Tp_Sense = NOMSET;   // For testing of change in value for shutdown function

  // Blynk  TODO:   these go inside loop?
  Blynk.run();
  blynk_timer_1.run(); blynk_timer_2.run(); blynk_timer_3.run(); blynk_timer_4.run(); 
  if (millis() - lastSync > ONE_DAY_MILLIS)
  {
    // Request time synchronization from the Particle Cloud once per day
    Particle.syncTime();
    lastSync = millis();
  }

  // Particle
  publishP  = ((now-lastPublishP) >= PUBLISH_PARTICLE_DELAY);
  if ( publishP ) lastPublishP  = now;

  // Read sensors
  // Stop every READ_TP_DELAY to read Tp, because it is corrupted by noise when running.
  // If Tp has changed since last
  if ( abs(Tp_Sense-last_Tp_Sense)>0.01 )
  {
    if ( debug>1 ) Serial.printf("TP:   Tp_Sense=%7.3f, last_Tp_Sense=%7.3f\n", Tp_Sense, last_Tp_Sense);
    last_Tp_Sense = Tp_Sense;
    lastReadTp = now;
  }
  readTp  = ( ((now-lastReadTp)>=READ_TP_DELAY)  || reset>0 );
  if ( readTp   ) lastReadTp   = now;
  dwellTp = ( (dwellTp && ((now-lastDwellTp)<DWELL_TP_DELAY)) || readTp );
  if ( !dwellTp ) lastDwellTp   = now;
  double deltaR = double(now - lastRead)/1000.;
  read    = ((now-lastRead) >= READ_DELAY || reset>0) && !publishP;
  if ( read     ) lastRead      = now;

  // Query Web
  query   = reset || (((now-lastQuery)>= QUERY_DELAY) && !read);
  if ( query    ) lastQuery     = now;

  // LCD display if used
  display   = ((now-lastDisplay) >= DISPLAY_DELAY) && !query;
  if ( display ) lastDisplay    = now;

  // Serial debug if used
  serial   = ((now-lastSerial) >= SERIAL_DELAY) && !query;
  if ( serial ) lastSerial    = now;

  // Control Demands
  past = now;
  now = millis();
  T = (now - past)/1e3;
  unsigned long deltaT = now - lastControl;
  //  control = (deltaT>=CONTROL_DELAY) && !display;
  control = (deltaT>=CONTROL_DELAY) || reset;
  if ( control  )
  {
    char  tempStr[23];  // time, year-mo-dyThh:mm:ss iso format, no time zone
    controlTime = decimalTime(&currentTime, tempStr);
    hmString    = String(tempStr);
    updateTime    = float(deltaT)/1000.0 + float(numTimeouts)/100.0;
    lastControl   = now;
  }
  delay(5);
  if ( bare )
  {
    delay ( bare_wait );
  }
  // Scheduling logic
  // 1.  Pot has highest priority
  //     a.  Pot will not hold past next schedule change
  //     b.  Web change will override it
  // 2.  Web Blynk has next highest priority
  //     a.  Web will hold only if HOLD is on
  //     b.  Web will HOLD indefinitely.
  //     c.  When Web is HELD, all other inputs are ignored
  // 3.  Finally the schedule gets it's say
  //     a.  Holds last number until time at next change
  //
  // Notes:
  // i.  webDmd is transmitted by Blynk to Photon only when it changes
  // ii. webHold is transmitted periodically by Blynk to Photon

  // Initialize scheduling logic - don't change on boot
  static int lastChangedPot = potValue;
  // static int lastChangedSched = schdDmd;
  // If user has adjusted the potentiometer (overrides schedule until next schedule change)
  // Use potValue for checking because it has more resolution than the integer potDmd
  if ( fabsf(potValue-lastChangedPot)>16 && checkPot )  // adjust from 64 because my range is 1214 not 4095
  {
      controlMode     = POT;
      double t = min(max(MINSET, potDmd), MAXSET);
      setSaveDisplayTemp(t);
      held = false;  // allow the pot to override the web demands.  HELD allows web to override schd.
      if ( debug>6 ) Serial.printf("Setpoint based on pot:  %f\n", t);
      lastChangedPot = potValue;
  }
  //
  // Otherwise if web Blynk has adjusted setpoint (overridden temporarily by pot, until next web adjust)
  // The held construct ensures that temp setting latched in by HOLD in Blynk cannot be accidentally changed
  // The webHold construct ensures that pushing HOLD in Blynk causes control to snap to the web demand
  else if ( ((abs(webDmd-lastChangedWebDmd)>0)  & (!held)) | (webHold & (webHold!=lastHold)) )
  {
    controlMode     = WEB;
    double t = min(max(MINSET, webDmd), MAXSET);
    setSaveDisplayTemp(t);
    lastChangedWebDmd   = webDmd;
  }
  else if ( !held )
  {
    controlMode = AUTO;
    double t = min(max(MINSET, NOMSET), MAXSET);
    setSaveDisplayTemp(t);
  }
  if ( webHold!=lastHold )
  {
    lastHold    = webHold;
    held        = webHold;
    saveTemperature(int(set), int(webDmd), held, EEPROM_ADDR);
  }
  if ( debug>3 )
  {
    if ( controlMode==AUTO ) Serial.printf("*******************Setpoint AUTO, set=%7.1f\n", set);
    else if ( controlMode==WEB ) Serial.printf("*******************Setpoint WEB, set=%7.1f\n", set);
    else if ( controlMode==POT ) Serial.printf("*******************Setpoint POT, set=%7.1f\n", set);
    else Serial.printf("*******************unknown controlMode %d\n", controlMode);
  }

  #ifndef NO_WEATHER_HOOK
    // Get OAT webhook
    if ( query    )
    {
      unsigned long           then = millis();     // Keep track of time
      getWeather();
      unsigned long           now = millis();     // Keep track of time
      if ( debug>0 ) Serial.printf("weather update=%f\n", float(now-then)/1000.0);
      if ( weatherGood )
      {
        OAT = tempf;
      }
      if ( debug>4 ) Serial.printf("OAT=%f at %s\n", OAT, hmString.c_str());
    }
  #endif

  // Outputs
  if ( control )
  {
    if ( !dwellTp )  // Freeze control if dwellTp
    {
      // err = set - Ta_Sense;
      err = set - Ta_Filt;
      double err_comp = DEAD(err, DB)*G;
      prop = max(min(err_comp * tau, 20), -20);   // TODO the prop limits do nothing
      integ = max(min(integ + updateTime*err_comp, pcnt_pot-prop), -prop);
      if ( (reset>0) & bare ) integ = 100;
      cont = max(min(integ+prop, pcnt_pot), 0);
    }

    cmd = max(min(min(pcnt_pot, cont),100.0), 0);

    // Latch on fan enable.   If temperature high, turn on.  If low, turn off.   If in-between and already on, leave on.
    // Latch prevents cycling of fan as Tp cools on startup of fan.
    if ( Tp_Sense>74.0 || ((Tp_Sense>73.0) & (duty>0)) ) duty = min(uint32_t(cmd*255.0/100.0), uint32_t(255));
    else duty = 0;
    if ( Time.hour(currentTime)<4 || Time.hour(currentTime)>23 ) duty = 0;
    if ( dwellTp ) duty = 0;
    pwm_write(duty);

    toggle = !toggle;
    digitalWrite(status_led, HIGH);
  }

  // Read sensors
  if ( read )
  {
    if ( debug>2 ) Serial.printf("Read update=%7.3f\n", deltaR);
    testing = load(reset, deltaR);
    if ( !bare )
    {
      testing = testing;
    }
    else
    {
      delay(41); // Usual I2C time
    }
  }

  // Publish to Particle cloud if desired (different than Blynk)
  // Visit https://console.particle.io/events.   Click on "view events on a terminal"
  // to get a curl command to run
  if ( (debug>0) & publishP )
  {
    if ( debug>3 ) Serial.println(F("publish"));
    publish_particle(now, publishP, cmd);
  }

  // Monitor for debug
  if ( debug>1 && serial )
  {
    serial_print_inputs(now, T);
    serial_print(cmd);
  }

  // Initialize complete once sensors and models started
  if ( read ) reset = 0;

} // loop


// Inputs serial print
void serial_print_inputs(unsigned long now, double T)
{
  Serial.print(F("0,")); Serial.print(now, DEC); Serial.print(", ");
  Serial.print(controlTime, 3); Serial.print(", ");
  Serial.print(T, 6); Serial.print(", ");  
  Serial.print(I2C_Status, DEC); Serial.print(", ");
  Serial.print(set, 1); Serial.print(", ");
  Serial.print(Tp_Sense, 1); Serial.print(", ");
  Serial.print(Ta_Sense, 1); Serial.print(", ");
  Serial.print(Ta_Filt, 1); Serial.print(", ");
  Serial.print(hum, 1); Serial.print(", ");
  Serial.print(pcnt_pot, 1); Serial.print(", ");
  Serial.print(OAT, 1); Serial.print(", ");
  Serial.print(solar_heat, 1); Serial.print(", ");
}

// Normal serial print
void serial_print(double cmd)
{
  if ( debug>0 )
  {
    Serial.print(cmd, 2); Serial.print(F(", "));
    Serial.print(duty, DEC); Serial.print(F(", "));
    Serial.println("");
  }
  else
  {
  }
  
}

// Load and filter
// TODO:   move 'read' stuff here
boolean load(int reset, double T)
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
    hum = roundf(rawHum / 163.83) + (HW_HUMCAL);
    int rawTemp = (Wire.read() << 6) & 0x3fc0;
    rawTemp |=Wire.read() >> 2;
    Ta_Sense = (float(rawTemp)*165.0/16383.0 - 40.0)*1.8 + 32.0 + (TA_TEMPCAL); // convert to fahrenheit and calibrate

    // Model
    duct->update(reset, T, Tp_Sense,  duty);
    room->update(reset, T, duct->Tdso(), duct->mdot_lag(), OAT, solar_heat, set);
    Ta_Obs = room->Ta();

    // MAXIM conversion 1-wire Tp plenum temperature
    if (sensor_plenum.read()) Tp_Sense = sensor_plenum.fahrenheit() + (TP_TEMPCAL);

    // Pot input
    int raw_pot_trim = analogRead(pot_trim);
    int raw_pot_control = analogRead(pot_control);
    pcnt_pot = min(max(double(raw_pot_trim)/40.96*1.1, double(raw_pot_control)/40.96*1.6), 100);

    // Tach input
    int raw_tach = analogRead(tach_sense);
    pcnt_tach = double(raw_tach)/40.96;
  }
  else
  {
    // Pots
    int raw_pot_control = analogRead(pot_control);
    Tp_Sense = double(raw_pot_control)/4096. * 10 + 70;;
    pcnt_pot = 100.;

    // Model
    duct->update(reset, T, Tp_Sense,  duty);
    room->update(reset, T, duct->Tdso(), duct->mdot_lag(), OAT, solar_heat, set);
    Ta_Obs = room->Ta();
    Ta_Sense = Ta_Obs;
  }
  Ta_Filt = TaSenseFilt->calculate( Ta_Sense, reset, T);

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


// Publish1 Blynk
void publish1(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write1\n");
    Blynk.virtualWrite(V0,  cmd);
    Blynk.virtualWrite(V2,  Ta_Sense);
    Blynk.virtualWrite(V3,  hum);
    //Blynk.virtualWrite(V4,  tempComp);
    Blynk.virtualWrite(V5,  Tp_Sense);
  #endif
}


// Publish2 Blynk
void publish2(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write2\n");
    Blynk.virtualWrite(V7,  held);
    Blynk.virtualWrite(V8,  updateTime);
    Blynk.virtualWrite(V9,  potDmd);
    Blynk.virtualWrite(V10, lastChangedWebDmd);
    Blynk.virtualWrite(V11, set);
  #endif
}


// Publish3 Blynk
void publish3(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write3\n");
    Blynk.virtualWrite(V12, schdDmd);
    Blynk.virtualWrite(V13, Ta_Sense);
    Blynk.virtualWrite(V14, I2C_Status);
    Blynk.virtualWrite(V15, hmString);
    Blynk.virtualWrite(V16, duty);
  #endif
}


// Publish4 Blynk
void publish4(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write4\n");
    Blynk.virtualWrite(V17, reco);
    Blynk.virtualWrite(V18, OAT);
    Blynk.virtualWrite(V19, Ta_Obs);
    Blynk.virtualWrite(V20, rejectHeat*200);
  #endif
}


// Check connection and publish Particle
void publish_particle(unsigned long now, bool publishP, double cmd)
{
  char  tmpsStr[STAT_RESERVE];
  sprintf(tmpsStr, "%s,%s,%18.3f,   %4.1f,%7.3f,%7.3f,%5.1f,   %5.2f,%4.1f,%7.3f,  %7.3f,%7.3f,%7.3f,%7.3f,%7.3f,%ld, %7.3f, %7.1f, %c", \
    unit.c_str(), hmString.c_str(), controlTime, callCount*1+set-HYST, Tp_Sense, Ta_Sense, cmd, updateTime, OAT, Ta_Obs, err, prop, integ, cont, pcnt_pot, duty, Ta_Filt, solar_heat,'\0');
  #ifndef NO_PARTICLE
    statStr = String(tmpsStr);
  #endif
  if ( debug>2 ) Serial.println(tmpsStr);
  if ( Particle.connected() )
  {
    if ( publishP ) 
    {
      if ( debug>2 ) Serial.printf("Particle write\n");
      unsigned nowSec = now/1000UL;
      unsigned sec = nowSec%60;
      unsigned min = (nowSec%3600)/60;
      unsigned hours = (nowSec%86400)/3600;
      sprintf(publishString,"%u:%u:%u",hours,min,sec);
      Particle.publish("Uptime",publishString);
      Particle.publish("stat", tmpsStr);
    }
  }
  else
  {
    if ( debug>1 ) Serial.printf("Particle not connected....connecting\n");
    Particle.connect();
    numTimeouts++;
  }
}


// Process a new temperature setting.   Display and save it.
int setSaveDisplayTemp(double t)
{
    set = t;
    // Serial.printf("setSave:   set=%7.1f\n", set);
    switch(controlMode)
    {
        case POT:   
           //displayTemperature(set);
           //displayCount=0;
           break;
        case WEB:   break;
        case SCHD:  break;
    }
    saveTemperature(set, int(webDmd), held, EEPROM_ADDR);
    return set;
}


#ifndef NO_BLYNK
// Attach a Slider widget to the Virtual pin 4 IN in your Blynk app
// - and control the web desired temperature.
// Note:  there are separate virtual IN and OUT in Blynk.
BLYNK_WRITE(V4) {
    if (param.asInt() > 0)
    {
        webDmd = param.asDouble();
    }
}
#endif


#ifndef NO_PARTICLE
int particleSet(String command)
{
  int possibleSet = atoi(command);
  if (possibleSet >= MINSET && possibleSet <= MAXSET)
  {
      webDmd = double(possibleSet);
      return possibleSet;
  }
  else return -1;
}
#endif


#ifndef NO_BLYNK
// Attach a switch widget to the Virtual pin 6 in your Blynk app - and demand continuous web control
// Note:  there are separate virtual IN and OUT in Blynk.
BLYNK_WRITE(V6) {
    webHold = param.asInt();
}
#endif
#ifndef NO_PARTICLE
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
#endif


// Save temperature setpoint to flash for next startup.   During power
// failures the thermostat will reset to the condition it was in before
// the power failure.   Filter initialized to sensed temperature (lose anticipation briefly
// following recovery from power failure).
void saveTemperature(const int set, const int webDmd, const int held, const int addr)
{
    uint8_t values[4] = { (uint8_t)set, (uint8_t)held, (uint8_t)webDmd, (uint8_t)(roundf(Ta_Obs)) };
    EEPROM.put(addr, values);
}


//Updates Weather Forecast Data
#ifndef NO_WEATHER_HOOK
void getWeather()
{
  if (debug>2)
  {
    Serial.print("Requesting Weather from webhook...");
    Serial.flush();
  }
  weatherGood = false;
  // publish the event that will trigger our webhook
  Particle.publish("get_weather");

  unsigned long wait = millis();
  //wait for subscribe to kick in or WEATHER_WAIT secs
  while ( !weatherGood && (millis()<wait+WEATHER_WAIT) )
  {
    //Tells the core to check for incoming messages from partile cloud
    Particle.process();
    delay(50);
  }
  if (!weatherGood)
  {
    if (debug>3) Serial.print("Weather update failed.  ");
    badWeatherCall++;
    if (badWeatherCall > 2)
    {
      //If 3 webhook calls fail in a row, Print fail
      if (debug>0) Serial.println("Webhook Weathercall failed!");
      badWeatherCall = 0;
    }
  }
  else
  {
    badWeatherCall = 0;
  }
} //End of getWeather function


// This function will get called when weather data comes in
void gotWeatherData(const char *name, const char *data)
{
  // Important note!  -- Right now the response comes in 512 byte chunks.
  //  This code assumes we're getting the response in large chunks, and this
  //  assumption breaks down if a line happens to be split across response chunks.
  //
  // Sample data:
  //  <location>Minneapolis, Minneapolis-St. Paul International Airport, MN</location>
  //  <weather>Overcast</weather>
  //  <temperature_string>26.0 F (-3.3 C)</temperature_string>
  //  <temp_f>26.0</temp_f>
  //  <visibility_mi>10.00</visibility_mi>
  String str          = String(data);
  String locationStr  = tryExtractString(str, "<location>",     "</location>");
  String weatherStr   = tryExtractString(str, "<weather>",      "</weather>");
  String tempStr      = tryExtractString(str, "<temp_f>",       "</temp_f>");
  String windStr      = tryExtractString(str, "<wind_string>",  "</wind_string>");
  String visStr       = tryExtractString(str, "<visibility_mi>","</visibility_mi>");

  if (locationStr != "" && debug>3) {
    if(debug>3) Serial.println("");
    Serial.println("At location: " + locationStr);
  }

  // Solar
  if ( weatherStr!= "" )  sun_wall->getWeather(weatherStr);
  if ( visStr!= "" )      sun_wall->getVisibility(visStr);
  solar_heat = sun_wall->solar_heat();
  if ( debug>3 ) Serial.printf("The weather is %d: %s, cover=%7.3f, visibility=%7.3f, solar heat = %7.3f\n",
        sun_wall->the_weather(), sun_wall->weatherStr().c_str(), sun_wall->cover(), sun_wall->visibility(), solar_heat);

  // Temperature
  if ( tempStr != "" )
  {
    weatherGood = true;
    #ifndef TESTING_WEATHER
      updateweatherhour = Time.hour();  // To check once per hour
    #endif
    tempf = atof(tempStr);
    if (debug>2)
    {
      if (debug<4) Serial.println("");
      Serial.println("The temp is: " + tempStr + String(" *F"));
      Serial.flush();
      Serial.printf("tempf=%f\n", tempf);
      Serial.flush();
    }
  }
  if (windStr != "" && debug>3) {
    Serial.println("The wind is: " + windStr);
  }
}


// Returns any text found between a start and end string inside 'str'
// example: startfooend  -> returns foo
String tryExtractString(String str, const char* start, const char* end)
{
  if (str == "")
  {
    return "";
  }
  int idx = str.indexOf(start);
  if (idx < 0)
  {
    return "";
  }
  int endIdx = str.indexOf(end);
  if (endIdx < 0)
  {
    return "";
  }
  return str.substring(idx + strlen(start), endIdx);
}
#endif


// Convert time to decimal for easy lookup
double decimalTime(unsigned long *currentTime, char* tempStr)
{
    Time.zone(GMT);
    *currentTime = Time.now();
    uint32_t year = Time.year(*currentTime);
    uint8_t month = Time.month(*currentTime);
    uint8_t day = Time.day(*currentTime);
    uint8_t hours = Time.hour(*currentTime);

    // Second Sunday Mar and First Sunday Nov; 2:00 am; crude DST handling
    if ( USE_DST)
    {
      uint8_t dayOfWeek = Time.weekday(*currentTime);     // 1-7
      if (  month>2   && month<12 &&
        !(month==3  && ((day-dayOfWeek)<7 ) && hours>1) &&  // <second Sunday Mar
        !(month==11 && ((day-dayOfWeek)>=0) && hours>0) )  // >=first Sunday Nov
        {
          Time.zone(GMT+1);
          *currentTime = Time.now();
        }
    }
    #ifndef FAKETIME
        uint8_t dayOfWeek = Time.weekday(*currentTime)-1;  // 0-6
        uint8_t minutes   = Time.minute(*currentTime);
        uint8_t seconds   = Time.second(*currentTime);
        if ( debug>5 ) Serial.printf("DAY %u HOURS %u\n", dayOfWeek, hours);
    #else
        // Rapid time passage simulation to test schedule functions
        uint8_t dayOfWeek = (Time.weekday(*currentTime)-1)*7/6;// minutes = days
        uint8_t hours     = Time.hour(*currentTime)*24/60; // seconds = hours
        uint8_t minutes   = 0; // forget minutes
        uint8_t seconds   = 0; // forget seconds
    #endif
    sprintf(tempStr, "%4u-%02u-%02uT%02u:%02u:%02u", int(year), month, day, hours, minutes, seconds);
    return ((((float(year-2021)*365.0+float(day))*24.0 + float(hours))*60.0 + float(minutes))*60.0 + \
                        float(seconds));
}


// Is leap year
int yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}


// Get day of year
int get_yday(int mon, int day, int year)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = yisleap(year);

    return days[leap][mon] + day;
}
