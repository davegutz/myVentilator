/*  Heart rate and pulseox calculation Constants

18-Dec-2020 	DA Gutz 	Created from MAXIM code.
// Copyright (C) 2020 - Dave Gutz
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

*/

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#if (PLATFORM_ID == 6)
#define PHOTON
#else
#undef PHOTON
#endif

// Disable flags if needed for debugging, usually commented
//#define BARE_PHOTON           // Run bare photon for testing.  Bare photon without this goes dark or hangs trying to write to I2C
//#define NO_WEATHER_HOOK       // Turn off webhook weather lookup.  Will get default OAT = 30F
//#define WEATHER_BUG           // Turn on bad weather return for debugging
//#define NO_BLYNK              // Turn off Blynk functions.  Interact using Particle cloud
//#define NO_CLOUD              // Turn off Particle cloud functions.  Interact using Blynk.
//#define BARE                  // Run without peripherals, except maybe a POT

// Test feature usually commented
//#define  FAKETIME                         // For simulating rapid time passing of schedule

// Constants always defined
const int8_t debug = 5;         // Level of debug printing (3)
#define TA_SENSOR 0x27          // Ambient room Honeywell temp sensor bus address (0x27)
#define TP_TEMPCAL 1            // Maxim 1-wire plenum temp sense calibrate (0), F
#define TA_TEMPCAL -7           // Honeywell calibrate temp sense (0), F
#define HW_HUMCAL -2            // Honeywell calibrate humidity sense (-2), %
#define ONE_DAY_MILLIS 86400000 // Number of milliseconds in one day (24*60*60*1000)
#define NOMSET 68               // Nominal setpoint for modeling etc, F
#define MINSET 50               // Minimum setpoint allowed (50), F
#define MAXSET 72               // Maximum setpoint allowed (72), F
#define CONTROL_DELAY    2000UL     // Control law wait, ms (5000)
#define MODEL_DELAY      5000UL     // Model wait, ms (5000)
#define PUBLISH_DELAY    30000UL    // Time between cloud updates, ms (30000UL)
#define PUBLISH_PARTICLE_DELAY 2000UL // Particle cloud updates (5000UL)
#define READ_DELAY       1000UL     // Sensor read wait (5000, 100 for stress test), ms (1000UL)
#define QUERY_DELAY      900000UL   // Web query wait (15000, 100 for stress test), ms (900000UL)
#define DISPLAY_DELAY    300UL      // Serial display scheduling frame time, ms (300UL)
#define FILTER_DELAY     5000UL     // In range of tau/4 - tau/3  * 1000, ms (5000UL)
#define SERIAL_DELAY     5000UL     // Serial print interval (5000UL)
#define STAT_RESERVE     150        // Space to reserve for status string publish (150)
#define HYST             0.01       // Heat control law hysteresis (0.75), F
#define WEATHER_WAIT     900UL      // Time to wait for weather webhook, ms (900UL)
#define GMT              -5         // Enter time different to zulu (does not respect DST)
#define USE_DST          1          // Whether to apply DST or not, 0 or 1
#define READ_TP_DELAY    1800000UL  // Time between Tp read shutdowns (1800000UL = 30 min)
#define DWELL_TP_DELAY   30000UL    // Time between Tp read shutdowns (30000UL = 30 sec)

#ifdef BARE
const boolean bare = true;  // Force continuous calibration mode to run with bare boards (false)
#else
const boolean bare = false;  // Force continuous calibration mode to run with bare boards (false)
#endif

const uint32_t pwm_frequency = 5000;    // Photon pwm driver frequency, Hz. (ECMF needs 1-10kHz)

#endif // CONSTANTS_H_
