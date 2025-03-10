#include <stdio.h>
#include "KepecsWheel.h"

// Initialize static member
RTC_DATA_ATTR uint32_t KepecsWheel::_logCount = 0;

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
        resetLogCount();
    }

    Serial.printf("Log count: %d\n", _logCount);
    pinMode(LED_BUILTIN, OUTPUT);

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

    // Initialize battery monitor with detailed debug
    if (!_batteryMonitor.begin(&Wire))
    {
        Serial.println("  Battery: failed to begin()");
        _isBatteryMonitorInitialized = false;
    }
    else
    {
        Serial.println("  Battery: begin() OK");
        _isBatteryMonitorInitialized = true;

        // Replace single delay with retry loop
        const uint8_t MAX_RETRIES = 10;
        uint8_t retries = 0;
        float voltage = 0;

        while (retries < MAX_RETRIES)
        {
            delay(1); // Shorter delay between attempts
            voltage = _batteryMonitor.cellVoltage();
            Serial.printf("  Battery init - attempt %d: %.2fV\n", retries + 1, voltage);

            if (voltage > 0 && !isnan(voltage))
            {
                break; // Valid reading obtained
            }
            retries++;
        }
    }

    allInitialized = _isSDInitialized && _isRTCInitialized && _isBatteryMonitorInitialized;
    if (!allInitialized)
    {
        _beginFailed = true;
    }
    return allInitialized;
}

bool KepecsWheel::reinit()
{
    return _isWakeFromSleep || _beginFailed;
}

bool KepecsWheel::logData()
{
    // Only log data if we woke from sleep
    if (!_isWakeFromSleep)
    {
        Serial.println("Not waking from sleep, skipping data logging");
        return false;
    }
    digitalWrite(LED_BUILTIN, HIGH);

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

    char datetime[20];
    snprintf(datetime, sizeof(datetime), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    char voltageStr[8];
    snprintf(voltageStr, sizeof(voltageStr), "%.2f", getBatteryVoltage());
    String dataString = String(datetime) + "," + String(voltageStr) + "," + String(_ulp.getEdgeCount() / 4);

    Serial.printf("\nLogging data: %s\n\n", dataString.c_str());

    // Write data
    bool success = dataFile.println(dataString);
    dataFile.close();
    incrementLogCount();
    digitalWrite(LED_BUILTIN, LOW);
    return success;
}

void KepecsWheel::sleep(int seconds)
{
    digitalWrite(LED_BUILTIN, LOW);
    if (_isBatteryMonitorInitialized)
    {
        _batteryMonitor.enableSleep(true); // Enable sleep capability
        _batteryMonitor.sleep(true);       // Enter sleep mode
    }
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

void KepecsWheel::resetLogCount()
{
    _logCount = 0;
}

void KepecsWheel::incrementLogCount()
{
    _logCount++;
}

uint32_t KepecsWheel::getLogCount()
{
    return _logCount;
}

bool KepecsWheel::shouldSync(int sleepSeconds, int syncMinutes)
{
    // Convert everything to minutes for comparison
    float elapsedMinutes = (float)(sleepSeconds * _logCount) / 60.0;
    Serial.printf("Sync check: elapsed=%.2f minutes, threshold=%d minutes\n", elapsedMinutes, syncMinutes);
    bool shouldSync = elapsedMinutes >= syncMinutes;
    if (shouldSync)
    {
        resetLogCount();
    }
    return shouldSync;
}

float KepecsWheel::getBatteryVoltage()
{
    return _batteryMonitor.cellVoltage();
}

float KepecsWheel::getBatteryPercent()
{
    return _batteryMonitor.cellPercent();
}