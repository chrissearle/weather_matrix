#include <Arduino.h>
#include <WiFiManager.h>
#include <Adafruit_GFX.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/TomThumb.h>

#include "timeutils.h"
#include "weather.h"

WiFiManager wm;
WiFiClient wifiClient;

TaskHandle_t refreshTask;

Weather *elveli;
Weather *blindern;

char currentClock[9] = {0};
char currentDate[11] = {0};
char currentDay[10] = {0};

bool seenClock = false;

const char *const dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

MatrixPanel_I2S_DMA matrix;

void refresh(void *parameter)
{
  const long freqElveli = 5 * 60 * 1000;
  const long freqBlindern = 3 * 60 * 1000;
  const long freqTime = 1 * 1000;

  long lastTime = millis() - freqTime;
  long lastElveli = millis() - freqElveli;
  long lastBlindern = millis() - freqBlindern;

  while (1)
  {
    long now = millis();

    if (now - lastTime > freqTime)
    {
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo))
      {
        Serial.println("Failed to obtain time");
      }
      else
      {
        sprintf(currentClock, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        sprintf(currentDate, "%02d/%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
        sprintf(currentDay, "%s", dayNames[timeinfo.tm_wday]);
      }
      lastTime = now;
      seenClock = true;
    }

    if (now - lastElveli > freqElveli)
    {
      elveli->update();
      lastElveli = now;
    }

    if (now - lastBlindern > freqBlindern)
    {
      blindern->update();
      lastBlindern = now;
    }

    // Watchdog
    delay(10);
  }
}

void setup()
{
  // Serial
  Serial.begin(115200);

  // WifiManager
  wm.autoConnect("WeatherMatrix");
  Serial.println("Connected to wifi");

  // Setup clock
  setupTime();

  elveli = new Weather("Elveli", &wifiClient, 60.023503, 10.618750);
  blindern = new Weather("Blindern", &wifiClient, 59.940395, 10.716017);

  xTaskCreatePinnedToCore(
      refresh,       /* Function to implement the task */
      "RefreshTask", /* Name of the task */
      10000,         /* Stack size in words */
      NULL,          /* Task input parameter */
      0,             /* Priority of the task */
      &refreshTask,  /* Task handle. */
      0);

  matrix.begin();
  matrix.setTextWrap(false);
}

void loop()
{

  if (seenClock)
  {
    matrix.clearScreen();

    matrix.setFont(&TomThumb);

    matrix.setCursor(1, 7);
    matrix.setTextColor(matrix.color565(255, 0, 255), matrix.color565(0, 0, 0));
    matrix.print(currentDay);

    matrix.setCursor(64 - 19 - 1, 7);
    matrix.setTextColor(matrix.color565(255, 0, 255), matrix.color565(0, 0, 0));
    matrix.print(currentDate);

    matrix.setFont();
    matrix.setTextColor(matrix.color565(255, 255, 0), matrix.color565(0, 0, 0));
    matrix.setCursor(15, 10);
    matrix.print(currentClock);

    delay(60000);
  }
  else
  {
    delay(1000);
  }
}
