#ifndef __CONTROLLER_CONFIG_STRUCT_H__
#define __CONTROLLER_CONFIG_STRUCT_H__

#include <mpr121_thresholds_struct.h>

/* version */

#define CONTROLLER_CONFIG_MAGIC 0x88
#define CONTROLLER_CONFIG_VERSION 0x01
#define CONTROLLER_HARDWARE_VERSION 0x01

/* config struct */
struct controller_config {
    uint8_t magic;            // 此值必须为 CONTROLLER_CONFIG_MAGIC
    uint8_t cfgVer;           // 配置文件版本
    uint8_t hwVer;            // 硬件版本
    uint8_t cfg0;             //
    uint8_t cfg1;             //
    uint8_t cfg2;             //
    uint8_t cfg3;             //
    uint16_t airMax;          // air判定上限
    uint16_t airMin;          // air判定下限
    mpr121Thresholds mpr[3];  // mpr模块阈值
    uint16_t mprFilter[3];    // mpr模块屏蔽区
    int16_t heightOffset[4];  // 高度偏移值
    uint8_t reserved[29];     //
    uint8_t xorSum;           // 前127字节异或和, 用于校验
};


#endif