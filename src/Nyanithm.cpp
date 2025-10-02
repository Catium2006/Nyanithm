
#include "pico/stdlib.h"
#include <stdio.h>

#define __NYANITHM_CPP__
#include <controller.hpp>

#include <controller_config.h>


int main() {

    sleep_ms(10);

    initUSBDevice();

    sleep_ms(300);

    readConfig();

    initDevices();

    mainLoop();
}
