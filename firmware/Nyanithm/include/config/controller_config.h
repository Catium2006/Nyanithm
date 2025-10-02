#ifndef __CONTROLLER_CONFIG_H__
#define __CONTROLLER_CONFIG_H__

#include <mpr121.h>
#include <controller_config_struct.h>

extern controller_config ControllerConfig;

void readConfig();
void setConfig();
void saveConfig();
bool checkConfigBuf(controller_config * config);
uint8_t getConfigPage();
void eraseConfigSector();

#endif