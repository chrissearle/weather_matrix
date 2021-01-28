#pragma once

#include <Arduino.h>
#include <WifiClient.h>

class Weather
{
private:
    const char *userAgent = "Arduino WeatherMatrix github.com/chrissearle/WeatherMatrix chris@chrissearle.org";
    const char *name;
    WiFiClient *client;
    double latitude;
    double longitude;
    char *url;

public:
    Weather(const char *name, WiFiClient *client, double latitude, double longitude) : name(name), client(client), latitude(latitude), longitude(longitude)
    {
        const char *format = "https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=%0.4lf&lon=%0.4lf";

        size_t needed = snprintf(NULL, 0, format, latitude, longitude);
        url = (char *)malloc(needed + 1);
        sprintf(url, format, latitude, longitude);
        Serial.print(name);
        Serial.print(": ");
        Serial.println(url);
    };
    void update();
};
