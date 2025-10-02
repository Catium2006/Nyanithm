#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include <tusb.h>

void initUSBDevice();
void stdio_init_cdc();

void hid_task();

#endif