#include <KepecsWheel.h>
#include <Hublink.h>

KepecsWheel wheel;
Hublink hublink(SD_CS);

int SLEEP_TIME_SECONDS = 10;
int SYNC_TIME_MINUTES = 60 * 4; // 4 hours
int SYNC_FOR_SECONDS = 30;

void onTimestampReceived(uint32_t timestamp)
{
  Serial.println("Received timestamp: " + String(timestamp));
  wheel.adjustRTC(timestamp);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  while (!wheel.begin())
  {
    // continue trying
    delay(1000);
  }
  wheel.logData();
  if (wheel.shouldSync(SLEEP_TIME_SECONDS, SYNC_TIME_MINUTES))
  {
    hublink.sync(SYNC_FOR_SECONDS);
  }
  wheel.sleep(SLEEP_TIME_SECONDS);
}

void loop()
{
}

void beginHublink()
{
  if (hublink.begin())
  {
    Serial.println("✓ Hublink.");
    hublink.setTimestampCallback(onTimestampReceived);

    // override default values with values from meta.json
    if (hublink.hasMetaKey("wheel", "sleep_time_seconds"))
    {
      SLEEP_TIME_SECONDS = hublink.getMeta<int>("wheel", "sleep_time_seconds");
      Serial.println("SLEEP_TIME_SECONDS: " + String(SLEEP_TIME_SECONDS));
    }
    if (hublink.hasMetaKey("wheel", "sync_time_minutes"))
    {
      SYNC_TIME_MINUTES = hublink.getMeta<int>("wheel", "sync_time_minutes");
      Serial.println("SYNC_TIME_MINUTES: " + String(SYNC_TIME_MINUTES));
    }
    if (hublink.hasMetaKey("wheel", "sync_for_seconds"))
    {
      SYNC_FOR_SECONDS = hublink.getMeta<int>("wheel", "sync_for_seconds");
      Serial.println("SYNC_FOR_SECONDS: " + String(SYNC_FOR_SECONDS));
    }
  }
  else
  {
    Serial.println("✗ Hublink Failed.");
  }
}