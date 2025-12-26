#pragma once

#include <stdint.h>
#include <driver/gpio.h>

// Define error 
typedef enum {
    DHT11_OK = 0,

    DHT11_ERR_NULL_PTR,
    DHT11_ERR_NOT_INITIALIZED,

    DHT11_ERR_TIMEOUT,
    DHT11_ERR_TIMEOUT_LOW,
    DHT11_ERR_TIMEOUT_HIGH,
    DHT11_ERR_TIMEOUT_BIT_START,
    DHT11_ERR_TIMEOUT_BIT_SYNC,
    DHT11_ERR_TIMEOUT_HIGH_PULSE,
    DHT11_ERR_TIMEOUT_HIGH_MEASURE,
    DHT11_ERR_CHECKSUM
} dht11_err_t;

// Define the struct for the sensor
typedef struct {
    gpio_num_t gpio;
    uint8_t humidity;
    uint8_t temperature;
} dht11_t;

dht11_err_t dht11_init(dht11_t *sensor, gpio_num_t gpio);
dht11_err_t dht11_read(dht11_t *sensor, uint8_t *temperature, uint8_t *humidity);


