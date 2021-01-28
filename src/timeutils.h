#pragma once

#include <Arduino.h>
#include <time.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setupTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
}
