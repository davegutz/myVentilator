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


#ifndef _MY_ROOM_H
#define _MY_ROOM_H


// Embedded model class
class RoomTherm
{
private:
  String name_;     // Object name label
  double Ha_;       // Air to wall constant, BTU/sec/F
  double Hc_;       // Core to air constant, BTU/sec/F
  double Hf_;       // Firing constant, BTU/sec/F
  double Ho_;       // Wall to outside constant, BTU/sec/F
  double Rn_;       // Low boiler reset curve OAT break, F
  double Rx_;       // High boiler reset curve OAT break, F
  double sNoise_;   // Noise sense level, F single amplitude
  double Ta_;       // Air temp, F
  double Ta_Sense_; // Sensed air temp (Ta_ with sNoise_), F
  double Tc_;       // Core heater temp, F
  double Tn_;       // Low boiler reset curve setpoint break, F
  double Tx_;       // High boiler reset curve setpoint break, F
  double Tw_;       // Outside wall temp, F
public:
  RoomTherm(void);
  RoomTherm(const String name, const double Ha, const double Hc, const double Hf, const double Ho, \
    const double Rn, const double Rx, const double Tn, const double Tx);
    RoomTherm(const String name, const double Ha, const double Hc, const double Hf, const double Ho, \
      const double Rn, const double Rx, const double Tn, const double Tx, const double sNoise);
  double update(const bool reset, const double T, const double temp,  const double duty, const double otherHeat, const double OAT);
  double Ta(void){return Ta_;};
  double Ta_Sense(void){return Ta_Sense_;};
  double Tc(void){return Tc_;};
  double Tw(void){return Tw_;};
};


#endif