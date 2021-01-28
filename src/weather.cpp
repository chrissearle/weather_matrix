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
    }
    else
    {
        Serial.print(name);
        Serial.print(" error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}
