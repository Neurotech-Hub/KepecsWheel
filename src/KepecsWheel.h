#ifndef KEPECS_WHEEL_H
#define KEPECS_WHEEL_H

#include <Arduino.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include "ULPManager.h"
#include "RTCManager.h"
#include "Preferences.h"

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
    uint32_t getWakeupCount();

private:
    const char *CSV_HEADER = "datetime,count,min_free_heap";
    String getCurrentFilename();
    bool createFile(String filename);
    void resetWakeupCount();
    void incrementWakeupCount();

    RTC_DATA_ATTR static uint32_t _wakeupCount; // Persists in RTC memory
    ULPManager _ulp;
    uint32_t _minFreeHeap; // Track minimum free heap
    bool _isWakeFromSleep;
    RTCManager _rtc;
    bool _isRTCInitialized;
    bool _isSDInitialized;
    bool allInitialized;
};

#endif // KEPECS_WHEEL_H