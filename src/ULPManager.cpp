#include "ULPManager.h"

// counts all state changes (LOW->HIGH, HIGH->LOW)
// divide by 2 for single transition type
const ulp_insn_t ulp_program[] = {
    // Initialize transition counter and previous state
    I_MOVI(R3, 0), // R3 <- 0 (reset the transition counter)
    I_MOVI(R2, 1), // R2 <- 0 (previous state, assume LOW initially)

    // Main loop
    M_LABEL(1),

    // Read RTC_GPIO_INDEX with RTC offset
    I_RD_REG(RTC_GPIO_IN_REG, RTC_GPIO_INDEX + RTC_GPIO_IN_NEXT_S, RTC_GPIO_INDEX + RTC_GPIO_IN_NEXT_S),

    // Save the current state in a temporary register (R1)
    I_MOVR(R1, R0), // R1 <- R0 (store current GPIO state temporarily)

    // Compare current state (R1) with previous state (R2)
    I_SUBR(R0, R1, R2), // R0 = current state (R1) - previous state (R2)
    I_BL(5, 1),         // If R0 == 0 (no state change), skip instructions
    I_ADDI(R3, R3, 1),  // Increment R3 by 1 (transition detected)
    I_MOVR(R2, R1),     // R2 <- R1 (store the current state for the next iteration)

    // Store the transition counter
    I_MOVI(R1, EDGE_COUNT), // Set R1 to address RTC_SLOW_MEM[1]
    I_ST(R3, R1, 0),        // Store it in RTC_SLOW_MEM

    // RTC clock on the ESP32-S3 is 17.5MHz, delay 0xFFFF = 3.74 ms
    I_DELAY(0xFFFF), // debounce
    I_DELAY(0xFFFF), // debounce
    I_DELAY(0xFFFF), // debounce
    I_DELAY(0xFFFF), // debounce
    I_DELAY(0xFFFF), // debounce
    I_DELAY(0xFFFF), // debounce

    M_BX(1), // Loop back to label 1
};

ULPManager::ULPManager()
{
    _initialized = false;
}

void ULPManager::begin()
{
    // init GPIO for ULP to monitor
    rtc_gpio_init(GPIO_SENSOR_PIN);
    rtc_gpio_set_direction(GPIO_SENSOR_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(GPIO_SENSOR_PIN);    // enable the pull-up resistor
    rtc_gpio_pulldown_dis(GPIO_SENSOR_PIN); // disable the pull-down resistor
    rtc_gpio_hold_en(GPIO_SENSOR_PIN);      // required to maintain pull-up
    _initialized = true;
    Serial.println("  ULP: initialization complete");
}

void ULPManager::start()
{
    Serial.println("  ULP: starting program");

    // Always reload the ULP program when starting
    size_t size = sizeof(ulp_program) / sizeof(ulp_insn_t);
    esp_err_t err = ulp_process_macros_and_load(PROG_START, ulp_program, &size);
    if (err != ESP_OK)
    {
        Serial.printf("  ULP: program load error: %d\n", err);
        return;
    }

    err = ulp_run(PROG_START);
    if (err != ESP_OK)
    {
        Serial.printf("  ULP: start error: %d\n", err);
        return;
    }
    Serial.println("  ULP: program started");
}

uint16_t ULPManager::getEdgeCount()
{
    uint16_t count = (uint16_t)(RTC_SLOW_MEM[EDGE_COUNT] & 0xFFFF);
    Serial.printf("  ULP: current edge count: %d\n", count);
    return count;
}

void ULPManager::clearEdgeCount()
{
    Serial.println("  ULP: clearing edge count");
    RTC_SLOW_MEM[EDGE_COUNT] = 0;
    Serial.printf("  ULP: verified count is now: %d\n", (uint16_t)(RTC_SLOW_MEM[EDGE_COUNT] & 0xFFFF));
}