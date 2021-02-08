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

#include "myRoom.h"

extern int debug;

// RoomTherm Class Functions
// Constructors
RoomTherm::RoomTherm()
  :   name_(""), Ha_(0), Hc_(0), Hf_(0), Ho_(0), Rn_(0), Rx_(0), Tn_(0), Tx_(0)
{}
RoomTherm::RoomTherm(const String name, const double Ha, const double Hc, const double Hf, const double Ho, \
  const double Rn, const double Rx, const double Tn, const double Tx)
  :   name_(name), Ha_(Ha), Hc_(Hc), Hf_(Hf), Ho_(Ho), Rn_(Rn), Rx_(Rx), Tn_(Tn), Tx_(Tx), sNoise_(0)
{}
RoomTherm::RoomTherm(const String name, const double Ha, const double Hc, const double Hf, const double Ho, \
    const double Rn, const double Rx, const double Tn, const double Tx, const double sNoise)
    :   name_(name), Ha_(Ha), Hc_(Hc), Hf_(Hf), Ho_(Ho), Rn_(Rn), Rx_(Rx), Tn_(Tn), Tx_(Tx), sNoise_(sNoise)
  {}
// Calculate
double RoomTherm::update(const bool reset, const double T, const double temp, const double duty, \
  const double otherHeat, const double OAT)
{
  // Three-state thermal model
  // The boiler in the house this was tuned to has a water reset schedule
  // that is a function of OAT.   If yours in constant, just
  // Tx to same value as Tn
  double Tb   = max(min((OAT-Rn_)/(Rx_-Rn_)*(Tx_-Tn_)+Tn_, Tn_), Tx_); // Curve interpolation
  // States
  if ( reset )
  {
    Ta_   = temp;                           // Air temp in house, F
    Tw_   = (OAT*Ho_+Ta_*Ha_)/(Ho_+Ha_);    // Outside wall temp, F
    Tc_   = (Ta_*(Ha_+Hc_)-Tw_*Ha_)/Hc_;    // Core heater temp, F
  }
  // Derivatives
  double dTw_dt   = -(Tw_-Ta_)*Ha_ - (Tw_-OAT)*Ho_;
  double dTa_dt   = -(Ta_-Tw_)*Ha_ - (Ta_-Tc_)*Hc_;
  double dTc_dt   = -(Tc_-Ta_)*Hc_ + duty*(Tb-Tc_)*Hf_ + otherHeat;
  if ( debug > 3 )
  {
    Serial.printf("%s:  dTw_dt=%7.3f, dTa_dt=%7.3f, dTc_dt=%7.3f\n", name_.c_str(), dTw_dt, dTa_dt, dTc_dt);
    Serial.printf("%s:  Tb=%7.3f, Tc=%7.3f, Ta=%7.3f, Tw=%7.3f, OAT=%7.3f\n", name_.c_str(), Tb, Tc_, Ta_, Tw_, OAT);
  }
  // Integration (Euler Backward Difference)
  Tw_  = min(max(Tw_+dTw_dt*T,  -40), 120);
  Ta_  = min(max(Ta_+dTa_dt*T,  -40), 120);
  Tc_  = min(max(Tc_+dTc_dt*T,  -40), 120);
  Ta_Sense_ = Ta_ + sNoise_ * float(random(-10, 10))/10.0;
  return dTa_dt;
}
