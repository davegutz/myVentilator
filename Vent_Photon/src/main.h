/*
 * Project Vent_Photon
  * Description:
  * Combine digital pot output in parallel with manual pot
  * to control an ECMF-150 TerraBloom brushless DC servomotor fan.
  * 
  * By:  Dave Gutz January 2021
  * 07-Jan-2021   A tinker version
  * 18-Feb-2021   Cleanup working version
  * 05-Jan-2023   Delete Blynk interface.   Pot sets everything now.   Dual range
  * 
//
// MIT License
//
// Copyright (C) 2023 - Dave Gutz
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
//
// See README.md
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

// Dependent includes.   Easier to debug code if remove unused include files
#include "myInsolation.h"
#include "mySync.h"
#include "mySubs.h"
#include "myCloud.h"

extern const int8_t debug = 2;  // Level of debug printing (3)
extern Publish pubList;
Publish pubList = Publish();
extern int badWeatherCall;      // webhook lookup counter
extern long updateweatherhour;  // Last hour weather updated
extern bool weatherGood;        // webhook OAT lookup successful, T/F
int badWeatherCall  = 0;
long updateweatherhour;         // Last hour weather updated
bool weatherGood;               // webhook OAT lookup successful, T/F

// Global locals
char buffer[256];               // Serial print buffer
int numTimeouts = 0;            // Number of Particle.connect() needed to unfreeze
String hmString = "00:00";      // time, hh:mm
double controlTime = 0.0;       // Decimal time, seconds since 1/1/2021
unsigned long lastSync = millis();// Sync time occassionally.   Recommended by Particle.
Pins *myPins;                   // Photon hardware pin mapping used


// Setup
void setup()
{
  // Serial
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  Serial.flush();
  delay(1000);          // Ensures a clean display on Arduino Serial startup on CoolTerm

  // Peripherals
  myPins = new Pins(D6, D2, D7, A1, A2, A3);
  if ( !bare )
  {
    // Status
    pinMode(myPins->status_led, OUTPUT);
    digitalWrite(myPins->status_led, LOW);

    // PWM Control
    pinMode(myPins->pwm_pin, OUTPUT);

    // I2C
    if ( !bare )
    {
      Wire.setSpeed(CLOCK_SPEED_100KHZ);
      Wire.begin();
    }
    // Initialize output
    pwm_write(0, myPins);
  }
  else
  {
    // Status
    pinMode(myPins->status_led, OUTPUT);
    digitalWrite(myPins->status_led, LOW);
  }

  // OAT
  // Lets listen for the hook response
  Particle.subscribe("hook-response/get_weather", gotWeatherData, MY_DEVICES);

  // Begin
  Particle.connect();
  // Particle.function("HOLD", particleHold);
  // Particle.function("SET",  particleSet);
  #ifdef PHOTON
    if ( debug>1 ) { sprintf(buffer, "Particle Photon.  bare = %d,\n", bare); Serial.print(buffer); };
  #else
    if ( debug>1 ) { sprintf(buffer, "Arduino Mega2560.  bare = %d,\n", bare); Serial.print(buffer); };
  #endif

  // Header for debug print
  if ( debug>1 )
  { 
    print_serial_header();
  }

  if ( debug>3 ) { Serial.print(F("End setup debug message=")); Serial.println(F(", "));};

} // setup


// Loop
void loop()
{
  static Sensors *sen = new Sensors(NOMSET, NOMSET, NOMSET, NOMSET, 32., 0, 50., 50., NOMSET, 0., 50., 50., POT, 0., 0., 0.);
  static Control *con = new Control(0.0, 0.0, 0, 0.0, 0.0, NOMSET, 0);
  static PID *pid = new PID(C_G/S_FANS, C_TAU, C_MAX, C_MIN, C_LLMAX, C_LLMIN, 0, 0, C_DB, 0, 0, 0);             // Main PID
  static PID *pid_o = new PID(C_G/S_FANS, C_TAU, C_MAX_O, C_MIN_O, C_LLMAX_O, C_LLMIN_O, 0, 0, C_DB_O, 0, 0, 0); // Observer PID
  static DuctTherm* duct = new DuctTherm("duct", M_AP_0, M_AP_1, M_AP_2, M_AQ_0, M_AQ_1, M_AQ_2, M_CPA, M_DUCT_DIA,
    M_DUCT_TEMP_DROP, M_GLKD, M_QLKD, M_MDOTL_DECR, M_MDOTL_INCR, M_MUA, M_RHOA, M_SMDOT*S_FANS, S_FANS);
  static RoomTherm* room = new RoomTherm("room", M_CPA, M_DN_TADOT, M_DN_TWDOT, M_GCONV,
    M_GLK, M_QLK, M_RSA, M_RSAI, M_RSAO, M_TRANS_CONV_LOW, M_TRANS_CONV_HIGH);
  static General2_Pole* TaSenseFilt = new General2_Pole(double(READ_DELAY)/1000., 0.05, 0.80, 0.0, 120.); // Sensor noise and general loop filter
  static Insolation* sun_wall = new Insolation(SUN_WALL_AREA, SUN_WALL_REFLECTIVITY, GMT);                // Solar insolation effects
  static DS18* sensor_plenum = new DS18(myPins->pin_1_wire);


  unsigned long currentTime;                // Time result
  static unsigned long now = millis();      // Keep track of time
  static unsigned long past = millis();     // Keep track of time
  static int reset = 1;                     // Dynamic reset
  double T = 0;                             // Present update time, s
  const int bare_wait = int(1);             // To simulate peripherals sample time
  // Synchronization
  bool publishP;                            // Particle publish, T/F
  static Sync *publishParticle = new Sync(PUBLISH_PARTICLE_DELAY);
  bool readTp;                              // Special sequence to read Tp affected by PWM noise with duty>0, T/F
  static Sync *readPlenum = new Sync(READ_TP_DELAY);
  static bool dwellTp;                      // Special hold to read Tp T/F
  static Sync *dwellPlenum = new Sync(DWELL_TP_DELAY);
  bool read;                                // Read, T/F
  static Sync *readSensors = new Sync(READ_DELAY);
  bool query;                               // Query schedule and OAT, T/F
  static Sync *queryWeb = new Sync(QUERY_DELAY);
  bool serial;                              // Serial print, T/F
  static Sync *serialDebug = new Sync(SERIAL_DELAY);
  bool control;                             // Control sequence, T/F
  static Sync *controlFrame = new Sync(CONTROL_DELAY);

  // ******************* Top of loop

  // Request time synchronization from the Particle Cloud once per day
  if (millis() - lastSync > ONE_DAY_MILLIS)
  {
    Particle.syncTime();
    lastSync = millis();
  }

  // Frame control
  // Stop every READ_TP_DELAY to read Tp, because it is corrupted by noise when running.
  // If Tp has changed since last
  if ( abs(sen->Tp-sen->last_Tp)>0.01 )
  {
    if ( debug>1 ) Serial.printf("TP:   Tp=%7.3f, last_Tp=%7.3f\n", sen->Tp, sen->last_Tp);
    sen->last_Tp = sen->Tp;
    readPlenum->update(now, true);
  }
  publishP = publishParticle->update(now, false);
  readTp = readPlenum->update(now, reset);
  dwellTp = dwellPlenum->updateN(now, false, readTp);
  read = readSensors->update(now, reset, !publishP);
  sen->T =  double(readSensors->updateTime())/1000.0;
  query = queryWeb->update(reset, now, !read);
  serial = serialDebug->update(false, now, !query);

  // Control References
  past = now;
  now = millis();
  T = (now - past)/1e3;
  control = controlFrame->update(reset, now, true);
  if ( control  )
  {
    char  tempStr[23];  // time, year-mo-dyThh:mm:ss iso format, no time zone
    controlTime = decimalTime(&currentTime, tempStr);
    hmString = String(tempStr);
    con->T = float(controlFrame->updateTime())/1000.0 + float(numTimeouts)/100.0;
  }
  delay(5);
  if ( bare )
  {
    delay ( bare_wait );
  }

  // Temperature setpoint logic.  Low end is direct pot control, high end is temp ref control
  if ( sen->pcnt_pot < POT_M_MAX )  // adjust from 64 because my range is 1214 not 4095
  {
    sen->controlMode = POT;
    double lim = min(max( (sen->pcnt_pot-POT_M_MAX)/(POT_M_MIN-POT_M_MAX)*(C_MAX-C_MIN)+C_MIN, C_MIN), C_MAX);
    setSaveDisplayTemp(lim, MINSET, sen, con);
  }
  else if ( sen->pcnt_pot > POT_A_MIN )
  {
    sen->controlMode = AUTO;
    double set = min(max( (sen->pcnt_pot-POT_A_MIN)/(POT_A_MAX-POT_A_MIN)*(MAXSET-MINSET)+MINSET, MINSET), MAXSET);
    setSaveDisplayTemp(C_MAX, set, sen, con);
  }
  else
  {
    sen->controlMode = NONE;
    setSaveDisplayTemp(C_MIN, MINSET, sen, con);
  }
  if ( debug>3 )
  {
    if ( sen->controlMode==AUTO ) Serial.printf("*******************Setpoint AUTO, lim%7.1f set%7.1f\n", con->lim, con->set);
    else if ( sen->controlMode==POT ) Serial.printf("*******************Setpoint POT, lim%7.1f set%7.1f\n", con->lim, con->set);
    else if ( sen->controlMode==NONE ) Serial.printf("*******************Setpoint NONE, lim%7.1f set%7.1f\n", con->lim, con->set);
    else Serial.printf("*******************unknown controlMode %d\n", sen->controlMode);
  }

  // Get OAT webhook and time it 
  if ( query    )
  {
    unsigned long then = millis();
    getWeather();
    unsigned long now = millis();
    if ( debug>0 ) Serial.printf("weather update=%f\n", float(now-then)/1000.0);
    if ( weatherGood )
    {
      if (pubList.weatherData.locationStr != "" && debug>3)
      {
        if(debug>3) Serial.println("");
        Serial.println("At location: " + pubList.weatherData.locationStr);
      }

      // Solar
      if ( pubList.weatherData.weatherStr!= "" )  sun_wall->getWeather(pubList.weatherData.weatherStr);
      if ( pubList.weatherData.visStr!= "" ) sun_wall->getVisibility(pubList.weatherData.visStr);
      if ( debug>3 ) Serial.printf("The weather is %d: %s, cover=%7.3f, visibility=%7.3f, solar heat = %7.3f\n",
        sun_wall->the_weather(), sun_wall->weatherStr().c_str(), sun_wall->cover(), sun_wall->visibility(), sun_wall->solar_heat());

      // Temperature
      if ( pubList.weatherData.tempStr != "" )
      {
        sen->OAT = atof(pubList.weatherData.tempStr);
        if (debug>2)
        {
          if (debug<4) Serial.println("");
          Serial.println("The temp is: " + pubList.weatherData.tempStr + String(" *F"));
          Serial.flush();
          Serial.printf("raw OAT=%f\n", sen->OAT);
          Serial.flush();
        }
      }
      if (pubList.weatherData.windStr != "" && debug>3)
      {
          Serial.println("The wind is: " + pubList.weatherData.windStr);
      }
    }
    if ( debug>4 ) Serial.printf("OAT=%f at %s\n", sen->OAT, hmString.c_str());
  }

  // Control
  if ( control )
  {

    if ( !dwellTp )  // Freeze control if dwellTp
    {

      // Main CLAW
      if ( sen->controlMode == AUTO )
      {
        pid->update((reset>0) & bare, con->set, sen->Ta_filt, con->T, 100., 100.);
        con->cmd = max(min(pid->cont, C_MAX), C_MIN);
      }
      else if (  sen->controlMode == POT )
      {
        pid->update(true, sen->Ta_filt, sen->Ta_filt, con->T, con->lim, con->lim);
        con->cmd = max(min(con->lim, C_MAX), C_MIN);
      }
      else
      {
        pid->update(true, sen->Ta_filt, sen->Ta_filt, con->T, con->lim, con->lim);
        con->cmd = C_MIN;
      }

      // Observer CLAW
      pid_o->update((reset>0) & bare, sen->Ta_filt, sen->Ta_obs, con->T, 0, C_MAX_O);
      con->cmd_o = max(min(pid_o->cont, C_MAX_O), C_MIN_O);
      if ( !bare ) con->heat_o = con->cmd_o * M_GAIN_O;
      else con->heat_o = 0;

    }

    // Latch on fan enable.   If temperature high, turn on.  If low, turn off.   If in-between and already on, leave on (hysteresis)
    // Latch prevents cycling of fan as Tp cools on startup of fan.
    if ( sen->Tp>TP_MIN_HI || ((sen->Tp>TP_MIN_LO) & (con->duty>0)) )
      con->duty = min( uint32_t(con->cmd*255.0/100.0), uint32_t(255) );
    else
      con->duty = 0;
    if ( Time.hour(currentTime)<TIME_ON || Time.hour(currentTime)>=TIME_OFF ) con->duty = 0;
    if ( dwellTp ) con->duty = 0;
    if ( sen->Tp>TP_FIRE ) con->duty = 0;  // Fire shutoff
    
    pwm_write(con->duty, myPins);
    if ( con->duty>0 ) digitalWrite(myPins->status_led, HIGH);
    else  digitalWrite(myPins->status_led, LOW);
  }

  // Read sensors
  if ( read )
  {
    if ( debug>2 ) Serial.printf("Read update=%7.3f\n", sen->T);
    load(reset, sen->T, sen, con, duct, room, TaSenseFilt, sun_wall, sensor_plenum, myPins);
    if ( bare ) delay(41);  // Usual I2C time
  }

  // Publish to Particle cloud if desired (different than Blynk)
  // Visit https://console.particle.io/events.   Click on "view events on a terminal"
  // to get a curl command to run
  if ( publishP || serial)
  {
    pubList.now = now;
    pubList.unit = unit;
    pubList.hmString =hmString;
    pubList.controlTime = controlTime;
    pubList.set = con->set;
    pubList.lim = con->lim;
    pubList.Tp = sen->Tp;
    pubList.Ta = sen->Ta;
    pubList.cmd = con->cmd;
    pubList.T = con->T;
    pubList.OAT = sen->OAT;
    pubList.Ta_obs = sen->Ta_obs;
    pubList.I2C_status = sen->I2C_status;
    pubList.err = pid->err;
    pubList.prop = pid->prop;
    pubList.integ = pid->integ;
    pubList.cont = pid->cont;
    pubList.pcnt_pot = sen->pcnt_pot;
    pubList.duty = con->duty;
    pubList.Ta_filt = sen->Ta_filt;
    pubList.solar_heat = sun_wall->solar_heat();
    pubList.heat_o = con->heat_o;
    pubList.hum = sen->hum;
    pubList.numTimeouts = numTimeouts;
    pubList.qduct = sen->qduct;
    pubList.mdot = sen->mdot;
    pubList.mdot_lag = sen->mdot_lag;

    // Publish to Particle cloud - how data is reduced by SciLab in ../dataReduction
    if ( publishP )
    {
      if ( debug>2 ) Serial.println(F("publish"));
      publish_particle(now);
    }

    // Monitor for debug
    if ( debug>0 && serial )
    {
      serial_print_inputs(now, T);
      serial_print(con->cmd);
    }

  }

  // Initialize complete once sensors and models started
  if ( read ) reset = 0;


} // loop
