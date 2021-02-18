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

#ifndef ARDUINO
#include "application.h" // Should not be needed if file .ino or Arduino
#endif
#include "mySubs.h"
#include "myInsolation.h"
#include "local_config.h"
#include "constants.h"
#include "myFilters.h"

extern const int8_t debug;
extern Insolation* sun_wall; 
extern char buffer[256];
extern String hmString;
extern double controlTime;
extern Sensors *sen;
extern Control *con;
extern PID *pid;
extern int numTimeouts;


// Check connection and publish Particle
void publish_particle(unsigned long now)
{
  sprintf(buffer, "%s,%s,%18.3f,   %4.1f,%7.3f,%7.3f,%5.1f,   %5.2f,%4.1f,%7.3f,  %7.3f,%7.3f,%7.3f,%7.3f,\
  %7.3f,%ld, %7.3f, %7.1f, %7.1f, %c", \
    unit.c_str(), hmString.c_str(), controlTime, con->set-HYST, sen->Tp, sen->Ta, con->cmd, con->T,
    sen->OAT, sen->Ta_obs, pid->err, pid->prop, pid->integ, pid->cont, sen->pcnt_pot, con->duty, sen->Ta_filt, sun_wall->solar_heat(), con->heat_o, '\0');
  
  if ( debug>2 ) Serial.println(buffer);
  if ( Particle.connected() )
  {
    if ( debug>2 ) Serial.printf("Particle write\n");
    unsigned nowSec = now/1000UL;
    unsigned sec = nowSec%60;
    unsigned min = (nowSec%3600)/60;
    unsigned hours = (nowSec%86400)/3600;
    char publishString[40];     // For uptime recording
    sprintf(publishString,"%u:%u:%u",hours,min,sec);
    Particle.publish("Uptime",publishString);
    Particle.publish("stat", buffer);
    if ( debug>2 ) Serial.println(buffer);
  }
  else
  {
    if ( debug>1 ) Serial.printf("Particle not connected....connecting\n");
    Particle.connect();
    numTimeouts++;
  }
}


// Inputs serial print
void serial_print_inputs(unsigned long now, double T)
{
  Serial.print(F("0,")); Serial.print(now, DEC); Serial.print(", ");
  Serial.print(controlTime, 3); Serial.print(", ");
  Serial.print(T, 6); Serial.print(", ");  
  Serial.print(sen->I2C_status, DEC); Serial.print(", ");
  Serial.print(con->set, 1); Serial.print(", ");
  Serial.print(sen->Tp, 1); Serial.print(", ");
  Serial.print(sen->Ta, 1); Serial.print(", ");
  Serial.print(sen->Ta_filt, 1); Serial.print(", ");
  Serial.print(sen->hum, 1); Serial.print(", ");
  Serial.print(sen->pcnt_pot, 1); Serial.print(", ");
  Serial.print(sen->OAT, 1); Serial.print(", ");
  Serial.print(sun_wall->solar_heat(), 1); Serial.print(", ");
  Serial.print(con->heat_o, 1); Serial.print(", ");
}
