#include <controller_config.h>
#include <hardware/flash.h>
#include <pico/flash.h>
#include <pico/multicore.h>
#include <pico/stdio.h>
#include <tusb.h>

#define FLASH_STORAGE_START (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

controller_config defalutConfig{
    .magic = CONTROLLER_CONFIG_MAGIC,                                    //
    .cfgVer = CONTROLLER_CONFIG_VERSION,                                 //
    .hwVer = CONTROLLER_HARDWARE_VERSION,                                //
    .cfg0 = 0x00,                                                        //
    .cfg1 = 0x00,                                                        //
    .cfg2 = 0x00,                                                        //
    .cfg3 = 0x00,                                                        //
    .airMax = 400,                                                       //
    .airMin = 200,                                                       //
    .mpr = { defaultThresholds, defaultThresholds, defaultThresholds },  //
    .mprFilter{ 0xffff, 0xffff, 0xffff },                                //
    .heightOffset = { 0, 0, 0, 0 },                                      //
    .xorSum = 0,                                                         // 前127字节异或和
};

controller_config ControllerConfig;

uint8_t config_buf[sizeof(controller_config)];

uint8_t currentPage = 0;

void saveConfigSafe(void* param) {
    if (currentPage == 15) {
        flash_range_erase(FLASH_STORAGE_START, FLASH_SECTOR_SIZE);
        currentPage = 0;
        flash_range_program(FLASH_STORAGE_START + (currentPage * FLASH_PAGE_SIZE), (const uint8_t*)&ControllerConfig, sizeof(controller_config));
    } else {
        currentPage++;
        flash_range_program(FLASH_STORAGE_START + (currentPage * FLASH_PAGE_SIZE), (const uint8_t*)&ControllerConfig, sizeof(controller_config));
    }
}

void saveConfig() {
    int result = flash_safe_execute(saveConfigSafe, nullptr, 10);
    printf("saveConfig result: %d, currentPage = %d\n", result, currentPage);
}

void eraseConfigSectorSafe(void* param) {
    flash_range_erase(FLASH_STORAGE_START, FLASH_SECTOR_SIZE);
    currentPage = 0;
}

void eraseConfigSector() {
    int result = flash_safe_execute(eraseConfigSectorSafe, nullptr, 10);
    printf("eraseConfig result: %d\n", result);
}

void readConfigSafe(void* param) {
    void* addr = (void*)(XIP_BASE + FLASH_STORAGE_START);
    bool found = false;
    void* target = nullptr;
    for (int page = 0; page < 16; page++) {
        controller_config* config = (controller_config*)addr;
        // printf("magic = %d\n", config->magic);
        // 检查是否为存储的配置
        if (config->magic == CONTROLLER_CONFIG_MAGIC) {
            found = true;
            target = addr;
            currentPage = page;
        } else {
            break;
        }
        addr += FLASH_PAGE_SIZE;
    }
    if (found) {
        // printf("found config in page %d\n", currentPage);
        memcpy(&ControllerConfig, target, sizeof(controller_config));

    } else {
        // printf("config not found\n");
        currentPage = 0xff;
        uint8_t* d = (uint8_t*)&defalutConfig;
        uint8_t sum = 0;
        for (int i = 0; i < sizeof(controller_config) - 1; i++) {
            sum ^= *d;
            d++;
        }
        defalutConfig.xorSum = sum;
        memcpy(&ControllerConfig, &defalutConfig, sizeof(controller_config));
        printf("using default config\n");
        saveConfigSafe(nullptr);
    }
}

void readConfig() {
    flash_safe_execute(readConfigSafe, nullptr, 10);
    // readConfigSafe(nullptr);
}

uint8_t getConfigPage() {
    return currentPage;
}

void setConfig() {
    int count = 0;
    for (; count < sizeof(controller_config); count++) {
        config_buf[count] = getchar();
    }
    if (checkConfigBuf((controller_config*)config_buf)) {
        printf("read %d byte. check ok. config changed.\n", count);
        memcpy(&ControllerConfig, config_buf, sizeof(controller_config));
    } else {
        printf("read %d byte. illegal config.\nconfig will not be changed\n", count);
    }
}

/**
 * 检查配置兼容性，校验数据完整性
 */
bool checkConfigBuf(controller_config* config) {
    if (config->magic != CONTROLLER_CONFIG_MAGIC) {
        return false;
    }
    if (config->hwVer != CONTROLLER_HARDWARE_VERSION) {
        return false;
    }
    if (config->cfgVer != CONTROLLER_CONFIG_VERSION) {
        return false;
    }
    uint8_t* ptr = (uint8_t*)config;
    uint8_t sum = 0x00;
    for (int i = 0; i < sizeof(controller_config) - 1; i++) {
        sum ^= *ptr;
        ptr++;
    }
    if (sum != config->xorSum) {
        return false;
    }
    return true;
}
