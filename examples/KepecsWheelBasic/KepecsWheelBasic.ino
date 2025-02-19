#include <KepecsWheel.h>

KepecsWheel wheel;

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
    wheel.sleep(10);
}

void loop()
{
}