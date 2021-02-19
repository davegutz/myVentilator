/*
 * Project Vent_Photon
  * Description:
  * Combine digital pot output in parallel with manual pot
  * to control an ECMF-150 TerraBloom brushless DC servomotor fan.
  * 
  * By:  Dave Gutz January 2021
  * 07-Jan-2021   A tinker version
  * 18-Feb-2021   Cleanup working version
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
#include "myInsolation.h"
#include "mySync.h"
#include "mySubs.h"

extern const int8_t debug = 2;         // Level of debug printing (3)
extern Publish pubList;
Publish pubList = Publish();

// Global locals
char buffer[256];           // Serial print buffer
int numTimeouts = 0;        // Number of Particle.connect() needed to unfreeze
String hmString = "00:00";  // time, hh:mm
double controlTime = 0.0;   // Decimal time, seconds since 1/1/2021
#ifndef NO_WEATHER_HOOK
  int badWeatherCall  = 0;  // webhook lookup counter
  long updateweatherhour;   // Last hour weather updated
  bool weatherGood;         // webhook OAT lookup successful, T/F
#endif
unsigned long lastSync = millis();// Sync time occassionally.   Recommended by Particle.
Pins *myPins;               // Photon hardware pin mapping used


// Local Utilities (use globals)
int particleHold(String command);
int particleSet(String command);
void gotWeatherData(const char *name, const char *data);
void getWeather(void);
void publish1(void);
void publish2(void);
void publish3(void);
void publish4(void);
BlynkTimer blynk_timer_1, blynk_timer_2, blynk_timer_3, blynk_timer_4;     // Time Blynk events


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

    // Initialize schedule
    saveTemperature(NOMSET, int(NOMSET), false, EEPROM_ADDR, NOMSET);

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
  #ifndef NO_WEATHER_HOOK
    Particle.subscribe("hook-response/get_weather", gotWeatherData, MY_DEVICES);
  #endif

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
    Serial.print(F("flag,time_ms,controlTime,T,I2C_status,set,Tp_Sense,Ta_Sense,Ta_filt,hum,pot,OAT,solar,heat_o,cmd,duty,")); Serial.println("");
  }

  if ( debug>3 ) { Serial.print(F("End setup debug message=")); Serial.println(F(", "));};

} // setup


// Loop
void loop()
{
  static Sensors *sen = new Sensors(NOMSET, NOMSET, NOMSET, NOMSET, 32, 0, 0, 0, NOMSET, 0,
                              NOMSET, 999, true, true, true, NOMSET, POT);                                      // Sensors
  static Control *con = new Control(0.0, 0.0, 0, 0.0, NOMSET, NOMSET, 0, NOMSET);                               // Control
  static PID *pid = new PID(C_G, C_TAU, C_MAX, C_MIN, C_LLMAX, C_LLMIN, 0, 0, C_DB, 0, 0, 0);                   // Main PID
  static PID *pid_o = new PID(C_G, C_TAU, C_MAX_O, C_MIN_O, C_LLMAX_O, C_LLMIN_O, 0, 0, C_DB_O, 0, 0, 0);       // Observer PID
  static DuctTherm* duct = new DuctTherm("duct", M_AP_0, M_AP_1, M_AP_2, M_AQ_0, M_AQ_1, M_AQ_2, M_DUCT_DIA,
    M_DUCT_TEMP_DROP, M_MDOTL_DECR, M_MDOTL_INCR, M_MUA, M_RHOA);                                               // Duct model
  static RoomTherm* room = new RoomTherm("room", M_CPA, M_DN_TADOT, M_DN_TWDOT, M_QCON, M_QLK, M_RSA, M_RSAI,
    M_RSAO, M_TRANS_CONV_LOW, M_TRANS_CONV_HIGH);                                                               // Room model
  static General2_Pole* TaSenseFilt = new General2_Pole(double(READ_DELAY)/1000., 0.05, 0.80, 0.0, 120.);       // Sensor noise and general loop filter
  static Insolation* sun_wall = new Insolation(SUN_WALL_AREA, SUN_WALL_REFLECTIVITY, GMT);                      // Solar insolation effects
  static DS18* sensor_plenum = new DS18(myPins->pin_1_wire);

  unsigned long currentTime;                // Time result
  static unsigned long now = millis();      // Keep track of time
  static unsigned long past = millis();     // Keep track of time
  static int reset = 1;                     // Dynamic reset
  double T = 0;                             // Present update time, s
  const int bare_wait = int(1);             // To simulate peripherals sample time
  bool checkPot = false;                    // Read the POT, T/F
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

  // Top of loop
  // Start Blynk
  Blynk.run(); blynk_timer_1.run(); blynk_timer_2.run(); blynk_timer_3.run(); blynk_timer_4.run(); 

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

  // Temperature setpoint logic
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
  // ii. sen->webHold is transmitted periodically by Blynk to Photon

  // Initialize scheduling logic - don't change on boot
  static int lastChangedPot = sen->potValue;
  // If user has adjusted the potentiometer (overrides schedule until next schedule change)
  // Use potValue for checking because it has more resolution than the integer potDmd
  if ( fabsf(sen->potValue-lastChangedPot)>16 && checkPot )  // adjust from 64 because my range is 1214 not 4095
  {
      sen->controlMode     = POT;
      double t = min(max(MINSET, sen->potDmd), MAXSET);
      setSaveDisplayTemp(t, sen, con);
      sen->held = false;  // allow the pot to override the web demands.  HELD allows web to override schd.
      if ( debug>6 ) Serial.printf("Setpoint based on pot:  %f\n", t);
      lastChangedPot = sen->potValue;
  }
  //
  // Otherwise if web Blynk has adjusted setpoint (overridden temporarily by pot, until next web adjust)
  // The held construct ensures that temp setting latched in by HOLD in Blynk cannot be accidentally changed
  // The sen->webHold construct ensures that pushing HOLD in Blynk causes control to snap to the web demand
  else if ( ((abs(con->webDmd-con->lastChangedWebDmd)>0)  & (!sen->held)) | (sen->webHold & (sen->webHold!=sen->lastHold)) )
  {
    sen->controlMode     = WEB;
    double t = min(max(MINSET, con->webDmd), MAXSET);
    setSaveDisplayTemp(t, sen, con);
    con->lastChangedWebDmd   = con->webDmd;
  }
  else if ( !sen->held )
  {
    sen->controlMode = AUTO;
    double t = min(max(MINSET, NOMSET), MAXSET);
    setSaveDisplayTemp(t, sen, con);
  }
  if ( sen->webHold!=sen->lastHold )
  {
    sen->lastHold = sen->webHold;
    sen->held = sen->webHold;
    saveTemperature(int(con->set), int(con->webDmd), sen->held, EEPROM_ADDR, sen->Ta_obs);
  }
  if ( debug>3 )
  {
    if ( sen->controlMode==AUTO ) Serial.printf("*******************Setpoint AUTO, set=%7.1f\n", con->set);
    else if ( sen->controlMode==WEB ) Serial.printf("*******************Setpoint WEB, set=%7.1f\n", con->set);
    else if ( sen->controlMode==POT ) Serial.printf("*******************Setpoint POT, set=%7.1f\n", con->set);
    else Serial.printf("*******************unknown controlMode %d\n", sen->controlMode);
  }

  #ifndef NO_WEATHER_HOOK
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
  #endif

  // Control and outputs
  if ( control )
  {
    if ( !dwellTp )  // Freeze control if dwellTp
    {
      // Main CLAW
      pid->update((reset>0) & bare, con->set, sen->Ta_filt, con->T, 100, sen->pcnt_pot);

      // Observer CLAW
      pid_o->update((reset>0) & bare, sen->Ta_filt, sen->Ta_obs, con->T, 0, C_MAX_O);

    }
    con->cmd = max(min(min(sen->pcnt_pot, pid->cont), C_MAX), C_MIN);
    con->cmd_o = max(min(pid_o->cont, C_MAX_O), C_MIN_O);
    if ( !bare ) con->heat_o = con->cmd_o * M_GAIN_O;
    else con->heat_o = 0;

    // Latch on fan enable.   If temperature high, turn on.  If low, turn off.   If in-between and already on, leave on.
    // Latch prevents cycling of fan as Tp cools on startup of fan.
    if ( sen->Tp>74.0 || ((sen->Tp>73.0) & (con->duty>0)) ) con->duty = min(uint32_t(con->cmd*255.0/100.0), uint32_t(255));
    else con->duty = 0;
    if ( Time.hour(currentTime)<4 || Time.hour(currentTime)>=23 ) con->duty = 0;
    if ( dwellTp ) con->duty = 0;
    if ( sen->Tp>110.0 ) con->duty = 0;  // Fire shutoff
    
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
    pubList.held = sen->held;
    pubList.potDmd = sen->potDmd;
    pubList.lastChangedWebDmd = con->lastChangedWebDmd;
    sen->webHold = pubList.webHold;
    con->webDmd = pubList.webDmd;

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


// Publish1 Blynk
void publish1(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write1\n");
    Blynk.virtualWrite(V0,  pubList.cmd);
    Blynk.virtualWrite(V2,  pubList.Ta);
    Blynk.virtualWrite(V3,  pubList.hum);
    // Blynk.virtualWrite(V4,  intentionally blank; used elsewhere);
    Blynk.virtualWrite(V5,  pubList.Tp);
  #endif
}


// Publish2 Blynk
void publish2(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write2\n");
    Blynk.virtualWrite(V7,  pubList.held);
    Blynk.virtualWrite(V8,  pubList.T);
    Blynk.virtualWrite(V9,  pubList.potDmd);
    Blynk.virtualWrite(V10, pubList.lastChangedWebDmd);
    Blynk.virtualWrite(V11, pubList.set);
  #endif
}


// Publish3 Blynk
void publish3(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write3\n");
    Blynk.virtualWrite(V12, pubList.solar_heat);
    Blynk.virtualWrite(V13, pubList.Ta);
    Blynk.virtualWrite(V14, pubList.I2C_status);
    Blynk.virtualWrite(V15, pubList.hmString);
    Blynk.virtualWrite(V16, pubList.duty);
  #endif
}


// Publish4 Blynk
void publish4(void)
{
  #ifndef NO_BLYNK
    if (debug>4) Serial.printf("Blynk write4\n");
    Blynk.virtualWrite(V17, false);
    Blynk.virtualWrite(V18, pubList.OAT);
    Blynk.virtualWrite(V19, pubList.Ta_obs);
    Blynk.virtualWrite(V20, pubList.heat_o);
  #endif
}


#ifndef NO_BLYNK
// Attach a Slider widget to the Virtual pin 4 IN in your Blynk app
// - and control the web desired temperature.
// Note:  there are separate virtual IN and OUT in Blynk.
BLYNK_WRITE(V4) {
    if (param.asInt() > 0)
    {
        pubList.webDmd = param.asDouble();
    }
}
#endif


#ifndef NO_PARTICLE
int particleSet(String command)
{
  int possibleSet = atoi(command);
  if (possibleSet >= MINSET && possibleSet <= MAXSET)
  {
    pubList.webDmd = double(possibleSet);
    return possibleSet;
  }
  else return -1;
}
#endif


#ifndef NO_BLYNK
// Attach a switch widget to the Virtual pin 6 in your Blynk app - and demand continuous web control
// Note:  there are separate virtual IN and OUT in Blynk.
BLYNK_WRITE(V6) {
    pubList.webHold = param.asInt();
}
#endif
#ifndef NO_PARTICLE
int particleHold(String command)
{
  if (command == "HOLD")
  {
    pubList.webHold = true;
    return 1;
  }
  else
  {
    pubList.webHold = false;
    return 0;
  }
}
#endif


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
  Particle.publish("get_weather");  // publish the event that will trigger our webhook

  unsigned long wait = millis();
  while ( !weatherGood && (millis()<wait+WEATHER_WAIT) ) //wait for subscribe to kick in or WEATHER_WAIT secs
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
  pubList.weatherData.locationStr = tryExtractString(str, "<location>", "</location>");
  pubList.weatherData.weatherStr = tryExtractString(str, "<weather>", "</weather>");
  pubList.weatherData.tempStr = tryExtractString(str, "<temp_f>", "</temp_f>");
  pubList.weatherData.windStr = tryExtractString(str, "<wind_string>", "</wind_string>");
  pubList.weatherData.visStr = tryExtractString(str, "<visibility_mi>", "</visibility_mi>");

  if ( pubList.weatherData.tempStr != "" )
  {
    weatherGood = true;
    #ifndef TESTING_WEATHER
      updateweatherhour = Time.hour();  // To check once per hour
    #endif
  }
}

#endif  // weather hook
