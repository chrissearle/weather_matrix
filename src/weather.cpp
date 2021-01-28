#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "weather.h"

void Weather::update()
{
    HTTPClient http;

    http.setUserAgent(userAgent);

    http.begin(url);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        const char *payload = http.getString().c_str();

        // Size from https://arduinojson.org/v6/assistant/
        DynamicJsonDocument doc(32768);
        deserializeJson(doc, payload);

        Serial.print(name);
        Serial.print(" updated : ");
        const char *updatedAt = doc["properties"]["meta"]["updated_at"];
        Serial.println(updatedAt);
        const char *timeSeriesName = getTimeSeriesName();

        JsonArray timeseries = doc["properties"]["timeseries"];

        temperature = 0.0;
        precipitation = 0.0;

        for (JsonVariant entry : timeseries)
        {
            if (strcmp(timeSeriesName, entry["time"]) == 0)
            {
                JsonObject details = entry["data"]["instant"]["details"];

                if (details.containsKey("air_temperature"))
                {
                    temperature = details["air_temperature"];
                }

                JsonObject nextDetails = entry["data"]["next_1_hours"]["details"];
                if (nextDetails.containsKey("precipitation_amount"))
                {
                    precipitation = nextDetails["precipitation_amount"];
                }
            }
        }
    }
    else
    {
        Serial.print(name);
        Serial.print(" error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

char *Weather::getTimeSeriesName()
{
    time_t tnow = time(nullptr);
    struct tm *timeinfo;

    char *buffer = new char[21];

    timeinfo = gmtime(&tnow);
    sprintf(buffer, "%04d-%02d-%02dT%02d:00:00Z", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour);

    return buffer;
}

double Weather::getTemperature()
{
    return temperature;
}

double Weather::getPrecipitation()
{
    return precipitation;
}
