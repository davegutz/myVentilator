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

extern int8_t debug;

// DuctTherm Class Functions

// Constructors
DuctTherm::DuctTherm()
    : name_(""), ap_0_(0), ap_1_(0), ap_2_(0), aq_0_(0), aq_1_(0), aq_2_(0),
    duct_dia_(0), duct_temp_drop_(0), mdot_lag_decr_(0),
    mdot_lag_incr_(0), mua_(0), rhoa_(0)
{}

DuctTherm::DuctTherm(const String name, const double ap_0, const double ap_1, const double ap_2,
    const double aq_0, const double aq_1, const double aq_2, const double duct_dia, const double duct_temp_drop,
    const double mdot_lag_decr, const double mdot_lag_incr, const double mua, const double rhoa, const double Smdot)
    : name_(name), ap_0_(ap_0), ap_1_(ap_1), ap_2_(ap_2), aq_0_(aq_0), aq_1_(aq_1), aq_2_(aq_2),
    duct_dia_(duct_dia), duct_temp_drop_(duct_temp_drop), mdot_lag_decr_(mdot_lag_decr),
    mdot_lag_incr_(mdot_lag_incr), mua_(mua), rhoa_(rhoa), Smdot_(Smdot)
{}
 
// Calculate
// Duct flow and temperature
double DuctTherm::flow_model_(const double fan_speed, const double rhoa, const double mua)
{
    cfm_ = Smdot_*(aq_2_ * fan_speed*fan_speed + aq_1_ * fan_speed + aq_0_);  // CFM
    mdot_ = cfm_ * rhoa_ * 60;   // lbm/hr
    press_ = ap_2_ * fan_speed*fan_speed + ap_1_ * fan_speed + ap_0_; // in H2O
    vel_ = cfm_ / (3.1415926 * duct_dia_*duct_dia_ / 4) * 60;        // ft/hr
    Re_d_ = rhoa_ * vel_ * duct_dia_ / mua_;
    return ( mdot_ );
}

// Heat balance and temperature
void DuctTherm::update(const int reset, const double T, const double Tp,  const uint32_t duty)
{
    // Inputs
    // Fan speed is linear with duty, per TerraBloom customer support.   Duty is 0-255.
    if ( reset>0 ) Tp_lagged_ = Tp;
    else
    {
        double delta = Tp - Tp_lagged_;
        Tp_lagged_ += T * delta / mdot_lag_incr_;
    }
    Tdso_ = Tp_lagged_ - duct_temp_drop_;
    mdot_ = flow_model_(double(duty)/2.55, rhoa_, mua_);

    // Lag
    if ( reset>0 ) mdot_lag_ = 933.;
    else
    {
        double delta = mdot_ - mdot_lag_;
        if ( delta>0 ) mdot_lag_ += T * delta / mdot_lag_incr_;
        else mdot_lag_ += T * delta / mdot_lag_decr_;
    }
}


// RoomTherm Class Functions

// Constructors
RoomTherm::RoomTherm()
  :   name_(""), cpa_(0), dn_tadot_(0), dn_twdot_(0), Gconv_(0), rsa_(0), rsai_(0),
  rsao_(0), Tk_(0), trans_conv_high_(0), trans_conv_low_(0)
{}

RoomTherm::RoomTherm(const String name, const double cpa, const double dn_tadot, const double dn_twdot, const double Gconv,
    const double qcon, const double qlk, const double rsa, const double rsai, const double rsao, const double Tk,
    const double trans_conv_low, const double trans_conv_high)
    : name_(name), cpa_(cpa), dn_tadot_(dn_tadot), dn_twdot_(dn_twdot), qcon_(qcon), qlk_(qlk), rsa_(rsa), rsai_(rsai),
    rsao_(rsao), Tk_(Tk), trans_conv_high_(trans_conv_high), trans_conv_low_(trans_conv_low)
{}

// Two-state thermal model
void RoomTherm::update(const int reset, const double T, const double Tdso, const double mdot, const double OAT, const double otherHeat, const double set)
{
    double qai;        // Heat into room air from duct flow, BTU/hr
    double qao;        // Heat out of room via the air displaced by duct flow, BTU/hr
    double qwi;        // Heat into wall from air, BTU/hr
    double qwo;        // Heat out of wall to OAT, BTU/hr

    // Input
    // qconv_ = (1. - max(min( (mdot - trans_conv_low_) / (trans_conv_high_ - trans_conv_low_), 1.), 0.)) * qcon_;

    // Flux
    if ( reset>0 )
    {
        // Ta_ = max((mdot*cpa_*rsa_*Tdso + OAT - qlk_*rsa_ + Tk_*Gconv_*rsa_) / (mdot*cpa_*rsa_ + 1. + Gconv_*rsa_), OAT);
        Ta_ = set;
        qai = (Ta_ - OAT) / rsa_;
        qao = qai;
        Tw_ = max(Ta_ - qai*rsai_, OAT);
    }
    else
    {
        qai = Tdso * cpa_ * mdot;
        qao = Ta_ * cpa_ * mdot;
    }
    qwi = (Ta_ - Tw_) / rsai_;
    qwo = (Tw_ - OAT) / rsao_;
    qconv_ = Gconv_*(Tk_ - Ta_);

    
    // Derivatives
    double Ta_dot = (qai - qao - qwi - qlk_ + qconv_ + otherHeat) / dn_tadot_;
    double Tw_dot = (qwi - qwo) / dn_twdot_;
    
    if ( debug > 3 )
    {
        Serial.printf("%s: reset=%d, mdot=%7.3f, Tdso=%7.3f, OAT=%7.3f,  ----->  Ta=%7.3f, Tw=%7.3f, \n", name_.c_str(), reset, mdot, Tdso, OAT, Ta_, Tw_);
        Serial.printf("%s: qai=%7.3f, qao=%7.3f, qwi=%7.3f, qwo=%7.3f, otherHeat=%7.3f, Ta_dot=%9.5f, Tw_dot=%9.5f,\n", name_.c_str(), qai, qao, qwi, qwo, otherHeat, Ta_dot, Tw_dot);
    }
    
    // Integration (Euler Backward Difference)
    Ta_  = min(max(Ta_ + T*Ta_dot,  -40), 120);
    Tw_  = min(max(Tw_ + T*Tw_dot,  -40), 120);
}
