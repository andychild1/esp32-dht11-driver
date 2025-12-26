#include "dht11.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"


#define DHT11_HANDSHAKE_RETRY  2

// Delay helper macro in uS
#define DHT11_DELAY_US(us)                          \
    do {                                            \
        int64_t _start = esp_timer_get_time();      \
        while (esp_timer_get_time() - _start < (us))\
            ;                                       \
    } while (0)


#define MEASURE_GPIO_HIGH_PULSE_US(gpio, max_us, duration_us, err) \
    do {                                                           \
        int64_t start = esp_timer_get_time();                      \
        while (gpio_get_level(gpio) == 1) {                        \
            if (esp_timer_get_time() - start > (max_us))           \
                return (err);                                      \
        }                                                          \
        (duration_us) = esp_timer_get_time() - start;              \
    } while (0)



static inline dht11_err_t wait_gpio_low(gpio_num_t gpio, uint32_t timeout_us, dht11_err_t err) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(gpio) != 1) {
        if (esp_timer_get_time() - start > timeout_us)
            return err;
    }
    return DHT11_OK;
}

static inline dht11_err_t wait_gpio_high(gpio_num_t gpio, uint32_t timeout_us, dht11_err_t err) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(gpio) != 0) {
        if (esp_timer_get_time() - start > timeout_us)
            return err;
    }
    return DHT11_OK;
}

// Initialize sensor
dht11_err_t dht11_init(dht11_t *sensor, gpio_num_t gpio) {
    if (sensor == NULL) return DHT11_ERR_NULL_PTR;
    sensor->gpio = gpio;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(gpio, 1);
    return DHT11_OK;
}

// Read from sensor
dht11_err_t dht11_read(dht11_t *sensor, uint8_t *temperature, uint8_t *humidity) {
    if (sensor == NULL || temperature == NULL || humidity == NULL) {
        return DHT11_ERR_NULL_PTR;
    }

    uint8_t data[5] = {0};
    gpio_num_t gpio = sensor->gpio;

    dht11_err_t result = DHT11_ERR_TIMEOUT;

    for (int attempt = 0; attempt < DHT11_HANDSHAKE_RETRY; attempt++) {
        // --- Start signal: MCU pulls low for >18ms ---
        gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
        gpio_set_level(gpio, 0);
        DHT11_DELAY_US(20000);          // 20ms low
        gpio_set_level(gpio, 1);
        DHT11_DELAY_US(30);             // 20-40µs high

        // --- Switch to input with internal pull-up ---
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
        gpio_pullup_en(gpio);
        gpio_pulldown_dis(gpio);

        // --- Wait for sensor response: 80µs low + 80µs high ---
        if (wait_gpio_low(gpio, 200, DHT11_ERR_TIMEOUT_LOW) != DHT11_OK ||
            wait_gpio_high(gpio, 200, DHT11_ERR_TIMEOUT_HIGH) != DHT11_OK) {
            // On failure: clean up pull-up and retry
            gpio_pullup_dis(gpio);
            vTaskDelay(10 / portTICK_PERIOD_MS);  // small delay before retry
            continue;
        }

        result = DHT11_OK;
        break;
    }

    if (result != DHT11_OK) {
        gpio_pullup_dis(gpio);
        return result;
    }

    // --- Read 40 bits ---
    for (int i = 0; i < 40; i++) {
        // Wait for start of bit: ~50µs low
        if (wait_gpio_low(gpio, 100, DHT11_ERR_TIMEOUT_BIT_START) != DHT11_OK) {
            result = DHT11_ERR_TIMEOUT_BIT_START;
            break;
        }

        // Measure duration of high pulse (26-28µs = 0, ~70µs = 1)
        uint32_t high_us;
        MEASURE_GPIO_HIGH_PULSE_US(gpio, 200, high_us, DHT11_ERR_TIMEOUT_HIGH_MEASURE);

        if (result != DHT11_OK) break;

        // Use 50µs as threshold (safer than 40µs)
        if (high_us > 50) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        } else {
            // bit remains 0
        }
    }

    // --- Cleanup: disable pull-up ---
    gpio_pullup_dis(gpio);

    if (result != DHT11_OK) {
        return result;
    }

    // --- Checksum verification ---
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        return DHT11_ERR_CHECKSUM;
    }

    // --- Extract values ---
    *humidity = data[0];      // Integer part only (DHT11)
    *temperature = data[2];   // Integer part only

    return DHT11_OK;
}


