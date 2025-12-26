# ESP32 DHT11 Driver

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.x-blue.svg)](https://docs.espressif.com/projects/esp-idf/en/latest/)
[![C](https://img.shields.io/badge/Language-C-green.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A simple, robust, and reliable driver for reading data from the **DHT11** temperature and humidity sensor on ESP32 using ESP-IDF.  
Implemented with pure bit-banging (no interrupts), optimized for stability and timing accuracy.

## Features

- Reliable reading of temperature and humidity (integer part)
- Automatic retry of the start signal
- Timeout detection and checksum validation
- Proper internal pull-up cleanup to prevent issues between consecutive readings
- Clean, commented, and easy-to-integrate code
- No external dependencies beyond ESP-IDF

## Compatibility

- ESP32 (all variants)
- ESP-IDF v4.4 or later (tested on v5.x)
- DHT11 sensor (also works with DHT12, but not optimized for DHT22)

## Wiring

| DHT11 Pin | ESP32             |
|-----------|-------------      |
| VCC       | 3.3V to 5V        |
| DATA      | GPIO rec. 4,18,19 |
| GND       | GND               |

**Note**: A 4.7kΩ–10kΩ pull-up resistor between DATA and VCC is strongly recommended (even though the driver uses the internal pull-up during reading).

## Installation

1. Copy the `dht11.c` and `dht11.h` files into your ESP-IDF project's `components/` or `main/` folder.
2. Include the header in your code:

```c
#include "dht11.h"
```

## Usage Example

```c
#include "dht11.h"
#include "esp_log.h"

static const char* TAG = "DHT11";

void app_main(void)
{
    dht11_t sensor;
    uint8_t temperature, humidity;

    // Initialize the sensor on GPIO 4
    if (dht11_init(&sensor, GPIO_NUM_4) != DHT11_OK) {
        ESP_LOGE(TAG, "DHT11 initialization error");
        return;
    }

    while (1) {
        if (dht11_read(&sensor, &temperature, &humidity) == DHT11_OK) {
            ESP_LOGI(TAG, "Humidity: %d%%  Temperature: %d°C", humidity, temperature);
        } else {
            ESP_LOGW(TAG, "DHT11 read error");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Read every 2 seconds (DHT11 requires at least 1s between readings)
    }
}