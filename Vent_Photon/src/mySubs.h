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


#ifndef _MY_SUBS_H
#define _MY_SUBS_H

// Sensors
struct Sensors
{
  double Ta;
  double Tp;
  double Ta_obs;
  double Ta_filt;
  double OAT;
  int I2C_status;
  double pcnt_pot;
  double pcnt_tach;
  double last_Tp;
  double T;
  double potValue;
  Sensors(void) {}
  Sensors(double Ta, double Tp, double Ta_obs, double Ta_filt,
    double OAT, int I2C_status, double pcnt_pot, double pcnt_tach, double last_Tp,
    double T, double potValue)
  {
    this->Ta = Ta;
    this->Tp = Tp;
    this->Ta_obs = Ta_obs;
    this->Ta_filt = Ta_filt;
    this->OAT = OAT;
    this->I2C_status = I2C_status;
    this->pcnt_pot = pcnt_pot;
    this->pcnt_tach = pcnt_tach;
    this->last_Tp = last_Tp;
    this->T = T;
    this->potValue = potValue;
  }
};

// Control
struct Control
{
  double cmd;
  double T;
  uint32_t duty;
  double cmd_o;
  Control(void) {}
  Control(double cmd, double T, uint32_t duty, double cmd_o)
  {
    this->cmd = cmd;
    this->T = T;
    this->duty = duty;
    this->cmd_o = cmd_o;
  }
};

#endif
