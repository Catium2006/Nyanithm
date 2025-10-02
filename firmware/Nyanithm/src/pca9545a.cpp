#include "pico/stdlib.h"
#include <pca9545a.h>

#include <debug.h>
#include <i2c_port.h>

#define HIGH true
#define LOW false

PCA9545A::PCA9545A(uint8_t _port, uint8_t i2c_addr, uint _gpio_reset) {
    port = _port;
    addr = i2c_addr;
    gpio_reset = _gpio_reset;
}

void PCA9545A::init() {
    DEBUG("pca9545.init() call");
    gpio_init(gpio_reset);
    gpio_set_dir(gpio_reset, true);
    gpio_pull_up(gpio_reset);
    gpio_put(gpio_reset, HIGH);
    sleep_ms(1);
    reset();
    // #ifdef ENABLE_DEBUG
    //     printf("read reg: 0x%02x\n", getReg());
    // #endif
}

void PCA9545A::reset() {
    DEBUG("pca9545.reset() call");
    gpio_put(gpio_reset, LOW);
    sleep_ms(1);
    gpio_put(gpio_reset, HIGH);
}

int PCA9545A::setChannel(uint8_t channel) {
    if (channel > 3) {
        return false;
    }
    uint8_t mask = 1 << channel;
    int result = i2c_write(port, addr, &mask, 1, false);
    return result;
}

uint8_t PCA9545A::getReg() {
    DEBUG("pca9545.getReg() call");
    uint8_t reg = 0;
    int result = i2c_read(port, addr, &reg, 1, false);
    // printf("result: %d\n", result);
    return reg;
}

int PCA9545A::setReg(uint8_t mask) {
    DEBUG("pca9545.setReg() call");
    int result = i2c_write(port, addr, &mask, 1, false);
    // printf("result: %d\n", result);
    return result != PICO_ERROR_GENERIC;
}

#undef HIGH
#undef LOW