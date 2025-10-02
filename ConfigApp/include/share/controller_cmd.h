#ifndef __CONTROLLER_CMD_H__
#define __CONTROLLER_CMD_H__

enum controller_command {
    CMD_CFG_READ = 0xA0,
    CMD_CFG_SAVE,
    CMD_CFG_ERASE,
    CMD_CFG_SET,
    CMD_MSR_IR,
    CMD_MSR_TOUCH,
    CMD_MSR_TIME,
    CMD_DEV_INIT,
    CMD_HID_STOP,
    CMD_HID_START,
    CMD_GET_STATUS,
    CMD_LOCATE_DEVICE,

};

#endif