#ifndef KEPECS_WHEEL_H
#define KEPECS_WHEEL_H

#include <Arduino.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "ULPManager.h"
#include "RTCManager.h"
#include "Preferences.h"
#include "Adafruit_MAX1704X.h"

#define SD_CS 10       // Chip-Select of SD card slot on RTC shield
#define LED_BUILTIN 13 // Built-in LED pin

class KepecsWheel
{
public:
    KepecsWheel();
    bool begin();
    bool logData();
    void sleep(int seconds);
    void adjustRTC(uint32_t timestamp);
    bool shouldSync(int sleepSeconds, int syncMinutes);
    uint32_t getLogCount();
    bool reinit();

private:
    const char *CSV_HEADER = "datetime,battery_voltage,count";
    String getCurrentFilename();
    bool createFile(String filename);
    void resetLogCount();
    void incrementLogCount();

    RTC_DATA_ATTR static uint32_t _logCount; // Persists in RTC memory
    ULPManager _ulp;
    bool _isWakeFromSleep;
    RTCManager _rtc;
    bool _isRTCInitialized;
    bool _isSDInitialized;
    bool allInitialized;
    bool _beginFailed = false;
    Adafruit_MAX17048 _batteryMonitor;
    float getBatteryVoltage();
    float getBatteryPercent();
    bool _isBatteryMonitorInitialized;
};

#endif // KEPECS_WHEEL_H