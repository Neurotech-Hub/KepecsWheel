#include <stdio.h>
#include "KepecsWheel.h"

// Initialize static member
RTC_DATA_ATTR uint32_t KepecsWheel::_wakeupCount = 0;

KepecsWheel::KepecsWheel()
{
    // Constructor implementation will be added as needed
}

bool KepecsWheel::begin()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    _isWakeFromSleep = (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER);
    Serial.printf("Wakeup reason: %d\n", wakeup_reason);

    // Reset counter on hard reset, increment on timer wakeup
    if (!_isWakeFromSleep)
    {
        resetWakeupCount();
    }
    else
    {
        incrementWakeupCount();
    }
    Serial.printf("Wakeup count: %d\n", _wakeupCount);

    SPI.begin(SCK, MISO, MOSI, SD_CS);
    if (SD.begin(SD_CS, SPI, 1000000))
    {
        Serial.println("SD Card initialized.");
        _isSDInitialized = true;
    }
    else
    {
        Serial.println("SD Card initialization failed.");
        _isSDInitialized = false;
    }

    Wire.begin();
    delay(10); // Give I2C time to stabilize

    // Initialize RTC
    if (!_rtc.begin())
    {
        Serial.println("  RTC: failed");
        _isRTCInitialized = false;
    }
    else
    {
        Serial.println("  RTC: OK");
        _isRTCInitialized = true;
    }

    allInitialized = _isSDInitialized && _isRTCInitialized;
    return allInitialized;
}

bool KepecsWheel::logData()
{
    // Only log data if we woke from sleep
    if (!_isWakeFromSleep)
    {
        Serial.println("Not waking from sleep, skipping data logging");
        return false;
    }

    String currentFile = getCurrentFilename();

    // Check if file exists, create it with header if it doesn't
    if (!SD.exists(currentFile))
    {
        if (!createFile(currentFile))
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000); // linger for a moment on error
            return false;
        }
    }

    // Open file in append mode
    File dataFile = SD.open(currentFile, FILE_APPEND);
    if (!dataFile)
    {
        Serial.println("Failed to open file for logging: " + currentFile);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000); // linger for a moment on error
        return false;
    }

    DateTime now = _rtc.now();
    _minFreeHeap = ESP.getMinFreeHeap();

    char datetime[20];
    snprintf(datetime, sizeof(datetime), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    String dataString = String(datetime) + "," +
                        String(_ulp.getEdgeCount()) + "," +
                        String(_minFreeHeap);

    Serial.println(dataString);

    // Write data
    bool success = dataFile.println(dataString);
    dataFile.close();
    return success;
}

void KepecsWheel::sleep(int seconds)
{
    digitalWrite(LED_BUILTIN, LOW);
    uint64_t microseconds = (uint64_t)seconds * 1000000ULL;
    esp_sleep_enable_timer_wakeup(microseconds);
    _ulp.clearEdgeCount();
    _ulp.begin();
    _ulp.start();
    esp_deep_sleep_start();
}

String KepecsWheel::getCurrentFilename()
{
    DateTime now = _rtc.now();
    char filename[20];
    snprintf(filename, sizeof(filename), "/WHEEL_%04d%02d%02d.csv",
             now.year(), now.month(), now.day());
    return String(filename);
}

bool KepecsWheel::createFile(String filename)
{
    // Ensure filename starts with a forward slash
    if (!filename.startsWith("/"))
    {
        filename = "/" + filename;
    }

    File file = SD.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to create file: " + filename);
        return false;
    }

    // Write header row
    if (file.println(CSV_HEADER))
    {
        Serial.println("Created new log file: " + filename);
        file.close();
        return true;
    }

    Serial.println("Failed to write header to file: " + filename);
    file.close();
    return false;
}

void KepecsWheel::adjustRTC(uint32_t timestamp)
{
    _rtc.adjustRTC(timestamp);
}

void KepecsWheel::resetWakeupCount()
{
    _wakeupCount = 0;
}

void KepecsWheel::incrementWakeupCount()
{
    _wakeupCount++;
}

uint32_t KepecsWheel::getWakeupCount()
{
    return _wakeupCount;
}

bool KepecsWheel::shouldSync(int sleepSeconds, int syncMinutes)
{
    // Convert everything to minutes for comparison
    float elapsedMinutes = (float)(sleepSeconds * _wakeupCount) / 60.0;
    return elapsedMinutes >= syncMinutes;
}