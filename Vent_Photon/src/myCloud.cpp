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

#include "application.h" // Should not be needed if file .ino or Arduino
#include "mySubs.h"
#include "myCloud.h"
#include "constants.h"
#include <math.h>

extern const int8_t debug;
extern Publish pubList;
extern char buffer[256];
extern int badWeatherCall;        // webhook lookup counter
extern long updateweatherhour;    // Last hour weather updated
extern bool weatherGood;          // webhook OAT lookup successful, T/F

// int particleSet(String command)
// {
//   int possibleSet = atoi(command);
//   if (possibleSet >= MINSET && possibleSet <= MAXSET)
//   {
//     return possibleSet;
//   }
//   else return -1;
// }


// int particleHold(String command)
// {
//   if (command == "HOLD")
//   {
//     return 1;
//   }
//   else
//   {
//     return 0;
//   }
// }


//Updates Weather Forecast Data
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
    updateweatherhour = Time.hour();  // To check once per hour
  }
}
