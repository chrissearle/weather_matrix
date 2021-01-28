#include <Arduino.h>
#include <WiFiManager.h>
#include <Adafruit_GFX.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/Org_01.h>
#include <Fonts/TomThumb.h>
#include <time.h>

#include "weather.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

WiFiManager wm;
WiFiClient wifiClient;

TaskHandle_t refreshTask;

Weather *elveli;
Weather *blindern;

char currentClock[9] = {0};
char currentDate[11] = {0};
char currentDay[10] = {0};

bool seenClock = false;
bool showWeather = false;
bool prevShowWeather = false;

const char *const dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

MatrixPanel_I2S_DMA matrix;

void syncClock()
{
  Serial.println("Syncing clock");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void refresh(void *parameter)
{
  const long freqElveli = 5 * 60 * 1000;
  const long freqBlindern = 5 * 60 * 1000;
  const long freqShowWeather = 20 * 1000;
  const long freqSync = 4 * 60 * 60 * 1000;

  long lastElveli = millis() - freqElveli;
  long lastBlindern = millis() - freqBlindern;
  long lastShowWeather = millis();
  long lastSync = millis();

  while (1)
  {
    long now = millis();

    if (now - lastSync > freqSync)
    {
      syncClock();
      lastSync = now;
    }

    if (now - lastShowWeather > freqShowWeather)
    {
      showWeather = !showWeather;
      lastShowWeather = now;
    }

    // Only update while clock is showing
    if (!showWeather)
    {
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
    }

    // Watchdog
    delay(10);
  }
}

int offsetForString(const char *string, int charWidth, int center)
{
  return center - ((strlen(string) * charWidth) / 2);
}

void setup()
{
  // Serial
  Serial.begin(115200);

  // WifiManager
  wm.autoConnect("WeatherMatrix");
  Serial.println("Connected to wifi");

  // Setup clock
  syncClock();

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

void updateClock()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    seenClock = false;
  }
  else
  {
    sprintf(currentClock, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    sprintf(currentDate, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    sprintf(currentDay, "%s", dayNames[timeinfo.tm_wday]);
    seenClock = true;
  }
}

void printTemp(double temp, int x, int y)
{
  if (temp < 0)
  {
    matrix.setTextColor(matrix.color565(127, 127, 255));
  }
  else if (temp < 10)
  {
    matrix.setTextColor(matrix.color565(255, 255, 127));
  }
  else
  {
    matrix.setTextColor(matrix.color565(255, 140, 0));
  }

  char tempStr[10];

  sprintf(tempStr, "%.1f C", temp);

  matrix.setCursor(offsetForString(tempStr, 3, 15) + x - 1, y);

  matrix.print(tempStr);
}

void printPrecipitation(double precipitation, int x, int y)
{
  if (precipitation < 0)
  {
    matrix.setTextColor(matrix.color565(127, 127, 255));
  }
  else
  {
    matrix.setTextColor(matrix.color565(127, 127, 127));
  }

  char precipStr[10];

  sprintf(precipStr, "%.1f mm", precipitation);

  matrix.setCursor(offsetForString(precipStr, 3, 15) + x - 1, y);

  matrix.print(precipStr);
}

void showWeatherScreen()
{
  uint16_t white = matrix.color565(127, 127, 127);
  matrix.fillRect(31, 0, 2, 32, white);
  matrix.drawLine(0, 8, 63, 8, white);

  matrix.setTextColor(matrix.color565(0, 255, 0), matrix.color565(0, 0, 0));

  matrix.setFont(&Org_01);

  matrix.setCursor(6, 5);
  matrix.print("Home");

  matrix.setCursor(37, 5);
  matrix.print("Elveli");

  matrix.setFont(&TomThumb);

  printTemp(blindern->getTemperature(), 0, 18);
  printTemp(elveli->getTemperature(), 32, 18);

  printPrecipitation(blindern->getPrecipitation(), 0, 27);
  printPrecipitation(elveli->getPrecipitation(), 32, 27);

  matrix.setFont();
}

void showClockScreen()
{
  if (seenClock)
  {
    matrix.setCursor(offsetForString(currentDay, 5, 28), 2);
    matrix.setTextColor(matrix.color565(255, 0, 255), matrix.color565(0, 0, 0));
    matrix.print(currentDay);

    matrix.setTextColor(matrix.color565(255, 255, 0), matrix.color565(0, 0, 0));
    matrix.setCursor(offsetForString(currentClock, 5, 28), 12);
    matrix.print(currentClock);

    matrix.setTextColor(matrix.color565(255, 0, 0), matrix.color565(0, 0, 0));
    matrix.setCursor(offsetForString(currentDate, 5, 28), 22);
    matrix.print(currentDate);
  }
}

void loop()
{
  updateClock();

  if (prevShowWeather != showWeather)
  {
    matrix.clearScreen();
    prevShowWeather = showWeather;
  }

  if (showWeather)
  {
    showWeatherScreen();
  }
  else
  {
    showClockScreen();
  }

  delay(1000);
}
