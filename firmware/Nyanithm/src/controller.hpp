#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

#ifndef __NYANITHM_CPP__
#warning this file should ONLY be included in Nyanithm.cpp
#endif

#include <debug.h>
#include <gpio_def.h>
#include <led.h>
#include <mpr121.h>
#include <pca9545a.h>
#include <stdio.h>
#include <usb_device.h>
#include <vl53l0x.h>

#include <i2c_config.h>
#include <i2c_port.h>

#include <controller_config.h>

#include <controller_cmd.h>

#include <pico/stdlib.h>

VL53L0X ir0(1, 0x29);
VL53L0X ir1(1, 0x29);
VL53L0X ir2(1, 0x29);
VL53L0X ir3(1, 0x29);

MPR121 mpr0(0, 0x5A);
MPR121 mpr1(0, 0x5B);
MPR121 mpr2(0, 0x5D);

PCA9545A pca9545a(1, 0x70, GPIO_PCA9545_RESET);

bool touchDeviceReady = false;
bool irDeviceReady = false;

void initIRReset() {
    gpio_init(GPIO_IR_RESET);
    gpio_set_dir(GPIO_IR_RESET, true);
    gpio_pull_up(GPIO_IR_RESET);
    gpio_put(GPIO_IR_RESET, HIGH);
    sleep_ms(1);
}

void resetIR() {
    gpio_put(GPIO_IR_RESET, LOW);
    sleep_ms(1);
    gpio_put(GPIO_IR_RESET, HIGH);
}
// bool ok = true;
void initIR() {

    initIRReset();
    resetIR();

    sleep_ms(10);

    pca9545a.setChannel(0);
    ir0.setTimeout(50);
    ir0.init();
    ir0.setMeasurementTimingBudget(25000);
    ir0.startContinuous(0);

    pca9545a.setChannel(1);
    ir1.setTimeout(50);
    ir1.init();
    ir1.setMeasurementTimingBudget(25000);
    ir1.startContinuous(0);

    pca9545a.setChannel(2);
    ir2.setTimeout(50);
    ir2.init();
    ir2.setMeasurementTimingBudget(25000);
    ir2.startContinuous(0);

    pca9545a.setChannel(3);
    ir3.setTimeout(50);
    ir3.init();
    ir3.setMeasurementTimingBudget(25000);
    ir3.startContinuous(0);

    pca9545a.setReg(0x00);
}

void initMPR121() {
    mpr0.init(MPR121_TOUCH_THRESHOLD_DEFAULT, MPR121_RELEASE_THRESHOLD_DEFAULT, true);
    mpr1.init(MPR121_TOUCH_THRESHOLD_DEFAULT, MPR121_RELEASE_THRESHOLD_DEFAULT, true);
    mpr2.init(MPR121_TOUCH_THRESHOLD_DEFAULT, MPR121_RELEASE_THRESHOLD_DEFAULT, true);

    mpr0.setThresholds(&ControllerConfig.mpr[0]);
    mpr1.setThresholds(&ControllerConfig.mpr[1]);
    mpr2.setThresholds(&ControllerConfig.mpr[2]);
}

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scanI2CBus(uint8_t port) {
    printf("I2C Bus Scan\n");
    uint8_t total_devices = 0;
    for (int addr = 0; addr < (1 << 7); addr++) {

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read(port, addr, &rxdata, 1, false);

        if (ret >= 0) {
            printf("device at 0x%02x\n", addr);
            total_devices++;
        }
    }
    printf("%d devices found\n", total_devices);
    printf("Done.\n");
}

void initDevices() {
    DEBUG("initDevices() call");
    initOnboardLED();
    DEBUG("initOnboardLED() done");
    // sleep_ms(50);

    initI2CBus(0, GPIO_I2C_0_SDA, GPIO_I2C_0_SCL, BR_I2C);
    initI2CBus(1, GPIO_I2C_1_SDA, GPIO_I2C_1_SCL, BR_I2C);
    DEBUG("initI2CBus() done");
    // sleep_ms(50);

    initMPR121();
    DEBUG("initMPR121() done");
    // sleep_ms(50);

    pca9545a.init();
    DEBUG("pca9545a.init() done");
    sleep_ms(10);

    initIR();
}

/**
 * TP0 TP1 TP2 TP3
 */
uint8_t touchData[4];

void updateTouch() {
    absolute_time_t t_s = get_absolute_time();
    uint16_t t0 = mpr0.touched();
    uint16_t t1 = mpr1.touched();
    uint16_t t2 = mpr2.touched();
    uint8_t mpr0l = (t0 & 0x0f) & MPR0L_FL;  // 4 bits
    uint8_t mpr0h = (t0 >> 4) & MPR0H_FL;    // 8 bits
    uint8_t mpr1l = t1 & MPR1L_FL;           // 8 bits
    uint8_t mpr1h = (t1 >> 8) & MPR1H_FL;    // 4 bits
    uint8_t mpr2l = t2 & MPR2L_FL;           // 8bits

    touchData[0] = mpr0h;
    touchData[1] = (mpr0l << 4) | mpr1h;
    touchData[2] = mpr1l;
    touchData[3] = mpr2l;
}

uint16_t heightData[4];
bool airKeys[6];

void updateAir() {
    bool updated = false;
    pca9545a.setChannel(0);
    if (ir0.readRangeContinuousMillimetersAsync(heightData + 0)) {
        heightData[0] += ControllerConfig.heightOffset[0];
        updated = true;
    }
    pca9545a.setChannel(1);
    if (ir1.readRangeContinuousMillimetersAsync(heightData + 1)) {
        heightData[1] += ControllerConfig.heightOffset[1];
        updated = true;
    }
    pca9545a.setChannel(2);
    if (ir0.readRangeContinuousMillimetersAsync(heightData + 2)) {
        heightData[2] += ControllerConfig.heightOffset[2];
        updated = true;
    }
    pca9545a.setChannel(3);
    if (ir3.readRangeContinuousMillimetersAsync(heightData + 3)) {
        heightData[3] += ControllerConfig.heightOffset[3];
        updated = true;
    }
    if (!updated) {
        return;
    }
    // 统计有效高度, 此处应为 dH = (..) / 6, 优化为dH = (..) * 21 / 128
    int16_t dH = ((ControllerConfig.airMax - ControllerConfig.airMin) * 21) >> 7;
    for (int j = 0; j < 6; j++) {
        bool detected = false;
        for (int i = 0; i < 4; i++) {
            if (ControllerConfig.airMin + dH * j <= heightData[i] && ControllerConfig.airMin + dH * (j + 1) >= heightData[i]) {
                detected = true;
                break;
            }
        }
        airKeys[j] = detected;
    }

    // printf("air: %d %d %d %d\n", heightData[0], heightData[1], heightData[2], heightData[3]);
}

uint16_t cycles = 0;
uint32_t time_s = 0;
uint32_t time_take = 0;

void calcCycle() {
    cycles++;
    if (cycles == 500) {
        cycles = 0;
        time_take = get_absolute_time() - time_s;
        time_s = get_absolute_time();
        time_take /= 500;
    }
}

void getStatus() {
    printf("Nyanithm build " __DATE__ " " __TIME__ "\n");
    printf("average %lu us used in reporting HID\n", time_take);
    printf("tof measure time: %llu\n", ir1.getAsyncMeasureTime());
    printf("using config in page %d\n", getConfigPage());
}

bool hid_work = true;

void handleCommand() {
    while (tud_cdc_available() > 0) {
        uint8_t cmd = getchar();

        if (cmd == CMD_CFG_READ) {
            readConfig();
            uint8_t* ptr = (uint8_t*)&ControllerConfig;
            for (int i = 0; i < sizeof(controller_config); i++) {
                putchar(*ptr);
                ptr++;
            }
            printf("read...\n");
            printf("using config in page %d\n", getConfigPage());
        } else if (cmd == CMD_CFG_ERASE) {
            printf("erase...\n");
            eraseConfigSector();
        } else if (cmd == CMD_CFG_SET) {
            // printf("set...");
            setConfig();
        } else if (cmd == CMD_CFG_SAVE) {
            printf("save...\n");
            saveConfig();
        } else if (cmd == CMD_DEV_INIT) {
            printf("init devices...\n");
            initDevices();
        } else if (cmd == CMD_HID_START) {
            printf("hid start...\n");
            hid_work = true;
        } else if (cmd == CMD_HID_STOP) {
            printf("hid stop...\n");
            hid_work = false;
        } else if (cmd == CMD_GET_STATUS) {
            printf("get status...\n");
            getStatus();
        } else if (cmd == CMD_LOCATE_DEVICE) {
            putchar(CONTROLLER_CONFIG_MAGIC);
        } else {
            printf("unknown command...\n");
        }
    }
}

void mainLoop() {

    while (true) {
        calcCycle();
        updateTouch();
        updateAir();
        handleCommand();
        // sleep_ms(10);
    }
}

#define ITF_KEYBOARD 0
// called by usb_device.cpp to run in core #1, while main codes run in core #0
void hid_task() {
    if (!hid_work) {
        return;
    }

    // Poll every 2ms
    const uint32_t interval_ms = 2;
    static uint32_t start_ms = 0;

    if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
        return;  // not enough time
    start_ms += interval_ms;

    // Remote wakeup
    if (tud_suspended()) {
        tud_remote_wakeup();
    }

    /*------------- Keyboard -------------*/
    if (tud_hid_n_ready(ITF_KEYBOARD)) {
        // use to avoid send multiple consecutive zero report for keyboard

        /**
         * report_buf
         * [0]
         * [1]
         * [2] A - H
         * [3] I - P
         * [4] Q - X
         * [5] Y, Z, 1 - 6
         * [6] 7 - 0, ENTER, ESCAPE, DELETE, TAB
         * [7] SPACE, -_, =+, [{, ]}, \|, [?], [?]
         */
        uint8_t report_buf[15];
        memset(report_buf, 0x00, sizeof(report_buf));
        // touch
        report_buf[2] = touchData[0];
        report_buf[3] = touchData[1];
        report_buf[4] = touchData[2];
        report_buf[5] = touchData[3];
        // air
        for (int i = 0; i < 6; i++) {
            if (airKeys[i]) {
                report_buf[9] |= (1 << i);
            }
        }
        tud_hid_report(0, report_buf, sizeof(report_buf));
    }
}

#endif