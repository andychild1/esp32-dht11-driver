#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"    
#include "esp_timer.h"    

#define DHT_PIN GPIO_NUM_18

void start()
{
    dht11_t sensor;
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    dht11_err_t err;

    printf("Initialization DHT11 on GPIO %d...\n", DHT_PIN);

    if (dht11_init(&sensor, DHT_PIN) != DHT11_OK) {
        printf("Error during DHT11 initialization!\n");
        return;
    }

    printf("DHT11 correctly initialized. Start readings...\n");

    while (1) {
        err = dht11_read(&sensor, &temperature, &humidity);
        
        if (err == DHT11_OK) {
            printf("Temperature: %d°C   Humidity: %d%%\n", temperature, humidity);
        } else if (err == DHT11_ERR_CHECKSUM) {
            printf("Error: Checksum invalid (corrupt data)\n");

        } else if (err == DHT11_ERR_TIMEOUT_LOW) {
            printf("Error: Timeout during LOW handshake\n");
        
        } else if (err == DHT11_ERR_TIMEOUT_HIGH) {
            printf("Error: Timeout during HIGH handshake\n");

        } else if (err == DHT11_ERR_TIMEOUT_BIT_SYNC) {
            printf("Error: Timeout during first 50uS LOW sync\n");

        } else if (err == DHT11_ERR_TIMEOUT_BIT_START) {
            printf("Error: Timeout during 50uS Bit reading\n");

        } else if (err == DHT11_ERR_TIMEOUT_HIGH_PULSE) {
            printf("Error: Timeout during HIGH pulse\n");

        } else if (err == DHT11_ERR_TIMEOUT_HIGH_MEASURE) {
            printf("Error: Timeout during HIGH pulse length reading\n");

        } else if (err == DHT11_ERR_NULL_PTR) {
            printf("Error: Protocol is failing\n");

        } else {
            printf("Unknow error during reading: %d\n", err);
        }

        // DHT11 requires at least 1 second between readings,
        // better 2 seconds for stability
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    start();
}

