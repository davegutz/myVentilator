#ifndef _myInsolation_H
#define _myInsolation_H

enum Conditions {FAIR, RAIN, OVERCAST, MOSTLY_CLOUDY, PARTLY_CLOUDY, CLEAR, A_FEW_CLOUDS,
                FOG_MIST, LIGHT_SNOW_FOG_MIST, RAIN_FOG_MIST,LIGHT_RAIN_FOG_MIST, HAZE,
                MIST, LIGHT_SNOW, SNOW, HEAVY_SNOW, UNKNOWN};

class Insolation
{
public:
  Insolation();
  Insolation(const double area);
  ~Insolation();
  // operators
  // functions
  double cover() { return(cover_); };
  void getVisibility(const String visStr);
  void getWeather(const String weatherStr);
  double turbidity() { return(turbidity_); };
  double visibility() { return(visibility_); };
  Conditions the_weather() { return(the_weather_); };
  String weatherStr() { return(weatherStr_); };
protected:
  double area_;
  double cover_;
  Conditions the_weather_;
  double turbidity_;
  double visibility_;
  String visStr_;
  String weatherStr_;
};





#endif