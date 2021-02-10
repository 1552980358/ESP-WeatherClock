#ifndef ESP_WEATHERCLOCK_METADATA_H
#define ESP_WEATHERCLOCK_METADATA_H

const char *ssid = "";
const char *passwd = "";

const String weather_id = "";
const String weather_appid = "";
const char *weather_url = ("http://api.openweathermap.org/data/2.5/weather?id=" + weather_id + "&appid=" + weather_appid + "&units=metric").c_str();

#endif //ESP_WEATHERCLOCK_METADATA_H
