#ifndef ULP_MANAGER_H
#define ULP_MANAGER_H

#include <Arduino.h>
#include "esp32s3/ulp.h"
#include "driver/rtc_io.h"
#include "soc/rtc_io_reg.h"
#include "ulp_common.h"

#define GPIO_SENSOR_PIN GPIO_NUM_18 // GPIO pin connected to the sensor
#define RTC_GPIO_INDEX 18           // attain dynamically with: rtc_io_number_get(GPIO_SENSOR_PIN)

enum
{
    EDGE_COUNT,
    PROG_START // Program start address
};

class ULPManager
{
public:
    ULPManager();
    void begin();
    void start();
    uint16_t getEdgeCount();
    void clearEdgeCount();

private:
    bool _initialized;
};

#endif // ULP_MANAGER_H
