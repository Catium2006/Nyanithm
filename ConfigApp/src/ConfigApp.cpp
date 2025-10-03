#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// ok it's fine
#include <windows.h>

#else
// ok it's fine
#error this project should be compiled in Windows
#endif

#include <fstream>
#include <iostream>
#include <vector>


using namespace std;

#include "controller_cmd.h"
#include "controller_config_struct.h"

controller_config ControllerConfig;

controller_config defaultConfig{
    CONTROLLER_CONFIG_MAGIC,                                      //
    CONTROLLER_CONFIG_VERSION,                                    //
    CONTROLLER_HARDWARE_VERSION,                                  //
    0x00,                                                         //
    0x00,                                                         //
    0x00,                                                         //
    0x00,                                                         //
    490,                                                          //
    250,                                                          //
    { defaultThresholds, defaultThresholds, defaultThresholds },  //
    { 0xffff, 0xffff, 0xffff },                                   //
    { 0, 0, 0, 0 },                                               //
    {},                                                           // reserved
    0                                                             // 前127字节异或和
};

void calcSum(controller_config* config) {
    uint8_t* ptr = (uint8_t*)config;
    uint8_t sum = 0x00;
    for (int i = 0; i < sizeof(controller_config) - 1; i++) {
        sum ^= *ptr;
        ptr++;
    }
    config->xorSum = sum;
}

bool checkSum(controller_config* config) {
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

void clear() {
    system("cls");
}

void nullOption() {
    // nothing to do
};

struct option {
    char ch;
    string function;
    void (*operation)(void);
};

void showCmd(vector<option> options) {
    options.push_back({ 'x', "返回" });
    while (true) {
        clear();
        cout << "Canithm Controller Config App" << endl << endl;
        cout << "按下按键并回车选择命令:" << endl;
        for (option opt : options) {
            cout << "  (" << opt.ch << ") " << opt.function << endl;
        }

        bool flag = false;
        char vk = 0;

        cin >> vk;
        if (vk == 'x') {
            break;
        }
        for (option opt : options) {
            if (vk == opt.ch) {
                flag = true;
                opt.operation();
            }
        }
        if (!flag) {
            cout << "未知操作, 请重试" << endl;
            Sleep(500);
        }
    }
}

#include <serialib.h>
bool opened;

serialib Serial;

bool findDevice() {
    for (int portNum = 1; portNum <= 32; portNum++) {
        // cout << "尝试开启端口 COM" << portNum << endl;
        char portName[32];
        sprintf(portName, "\\\\.\\COM%d", portNum);
        char errorOpenning = Serial.openDevice(portName, 115200, SERIAL_DATABITS_8, SERIAL_PARITY_NONE, SERIAL_STOPBITS_1);
        if (errorOpenning != 1) {
            continue;
        } else {
            Sleep(100);
            cout << "打开端口 COM" << portNum << endl;
            uint8_t no_use[1024];
            Serial.DTR(1);
            // 先清空数据
            Serial.flushReceiver();
            char buf;
            Serial.writeChar(CMD_LOCATE_DEVICE);
            Sleep(100);
            // Serial.readChar(&buf, 500);
            Serial.readBytes(&buf, 1, 100, 100);

            uint8_t reply = buf;
            printf("0x%02x\n", buf);
            if (reply == CONTROLLER_CONFIG_MAGIC) {
                cout << "检测到设备" << endl;
                opened = true;
                break;
            } else {
                Serial.closeDevice();
            }
        }
    }
    Sleep(1000);
}

void setPort() {
    clear();
    cout << "自动连接设备" << endl;
    if (opened) {
        cout << "已经连接到设备" << endl;
    } else {
        findDevice();
        if (opened) {
            cout << "已连接" << endl;
        } else {
            cout << "未找到设备, 请重试" << endl;
        }
    }
    Sleep(1000);
}

void readConfig() {
    char buf[1024];
    Serial.writeChar(CMD_CFG_READ);
    Sleep(200);
    Serial.readBytes(buf, 1024, 50);

    memcpy(&ControllerConfig, buf, sizeof(controller_config));

    cout << (char*)(buf + sizeof(controller_config)) << endl;
    Sleep(1000);
}

void readStatus() {
    char buf[1024];
    Serial.writeChar(CMD_GET_STATUS);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

void saveConfig() {
    char buf[1024];
    Serial.writeChar(CMD_CFG_SAVE);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

void eraseFlash() {
    char buf[1024];
    Serial.writeChar(CMD_CFG_ERASE);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

void initDevice() {
    char buf[1024];
    Serial.writeChar(CMD_DEV_INIT);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

void stopHID() {
    char buf[1024];
    Serial.writeChar(CMD_HID_STOP);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

void startHID() {
    char buf[1024];
    Serial.writeChar(CMD_HID_START);
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    // system("pause");
    Sleep(1000);
}

vector<option> deviceAdvancedFunctionOptions = {
    { '1', "从内置flash读取配置,", readConfig },               //
    { '2', "读取设备信息", readStatus },                       //
    { '3', "保存设备正在使用的配置到内置flash", saveConfig },  //
    { 'a', "擦除内置flash", eraseFlash },                      //
    { 'b', "重新初始化设备", initDevice },                     //
    { 'c', "停止设备HID上报", stopHID },                       //
    { 'd', "启动设备HID上报", startHID },                      //
};

void deviceAdvancedFunction() {
    if (opened) {
        showCmd(deviceAdvancedFunctionOptions);
    } else {
        cout << "请先连接设备" << endl;
        Sleep(1000);
    }
}

void loadDefaultConfig() {
    memcpy(&ControllerConfig, &defaultConfig, sizeof(controller_config));
}

void readConfigBin() {
    ifstream ifs;
    ifs.open("config.bin", ios::in | ios::binary);
    if (ifs.good()) {
        ifs.read((char*)&ControllerConfig, sizeof(controller_config));
        cout << "加载完毕" << endl;
    } else {
        cout << "无法打开 config.bin" << endl;
    }
    Sleep(1000);
}

void setConfig() {
    char buf[1024];
    Serial.writeChar(CMD_CFG_SET);
    Serial.writeBytes(&ControllerConfig, sizeof(controller_config));
    Sleep(200);
    Serial.readBytes(buf, 1024, 10);
    cout << buf << endl;
    cout << "正在保存..." << endl;
    saveConfig();
}

void setConfigBin() {
    ofstream ofs;
    ofs.open("config.bin", ios::out | ios::binary);
    if (ofs.good()) {
        ofs.write((const char*)&ControllerConfig, sizeof(controller_config));
        ofs.flush();
        cout << "保存到 config.bin" << endl;
    } else {
        cout << "无法写入文件" << endl;
    }
    Sleep(1000);
}

void modifyTouch() {
    clear();
    cout << "正在进行触摸灵敏度调整" << endl;
    cout << "模块编号, 通道与键位的映射另请查阅用户手册" << endl;
    cout << "输入待调整的 MPR 模块编号 (0 - 2)" << endl;
    int mprNum;
    cin >> mprNum;
    if (mprNum > 2 || mprNum < 0) {
        cout << "编号无效 (" << mprNum << ")" << endl;
        Sleep(1000);
        return;
    }
    cout << "输入待调整的 MPR 通道 (0 - 11)" << endl;
    int channel;
    cin >> channel;
    if (channel > 11 || channel < 0) {
        cout << "通道无效(" << channel << ")" << endl;
        Sleep(1000);
        return;
    }
    int buf;
    cout << "输入触发阈值, 当前值 = " << (int)ControllerConfig.mpr[mprNum].touch[channel] << " (0 - 255)" << endl;
    cin >> buf;
    ControllerConfig.mpr[mprNum].touch[channel] = buf;
    cout << "输入释放阈值, 当前值 = " << (int)ControllerConfig.mpr[mprNum].release[channel] << " (0 - 255)" << endl;
    cin >> buf;
    ControllerConfig.mpr[mprNum].release[channel] = buf;
    cout << "设置成功" << endl;
    calcSum(&ControllerConfig);
    Sleep(1000);
}

void modifyAir() {
    clear();
    cout << "正在进行 Air 判定调整" << endl;
    int aMax, aMin;
    cout << "输入判定下限, 当前值 = " << ControllerConfig.airMin << " mm (0 - 1000)" << endl;
    cin >> aMin;
    cout << "输入判定上限, 当前值 = " << ControllerConfig.airMax << " mm (0 - 1000)" << endl;
    cin >> aMax;
    ControllerConfig.airMax = aMax;
    ControllerConfig.airMin = aMin;
    calcSum(&ControllerConfig);
    cout << "设置成功" << endl;
    Sleep(1000);
}

void modifyAirOffest() {
    clear();
    cout << "正在校准 Air 传感器偏移量" << endl;
    cout << "从左到右依次为 0 1 2 3" << endl;
    cout << "输入待调整的传感器编号 (0 - 3)" << endl;
    int num;
    cin >> num;
    if (num > 3 || num < 0) {
        cout << "编号无效 (" << num << ")" << endl;
        Sleep(1000);
        return;
    }
    cout << "输入偏移量, 当前值 = " << (int)ControllerConfig.heightOffset[num] << " mm (0 - 255)" << endl;
    int buf;
    cin >> buf;
    ControllerConfig.heightOffset[num] = buf;
    cout << "设置成功" << endl;
    calcSum(&ControllerConfig);
    Sleep(1000);
}

vector<option> modifyConfigOptions = {
    { '1', "调整触摸灵敏度", modifyTouch },       //                         //
    { '2', "调整 Air 判定范围", modifyAir },      //                         //
    { '3', "校准 Air 偏移量", modifyAirOffest },  //                         //
};

void modifyConfig() {
    showCmd(modifyConfigOptions);
}

void checkConfigData() {
    if (checkSum(&ControllerConfig)) {
        cout << "校验成功" << endl;
    } else {
        cout << "校验失败" << endl;
    }
    Sleep(1000);
}

vector<option> changeConfigOptions = {
    { '1', "预加载默认配置 (不写入设备)", loadDefaultConfig },  //
    { '2', "预加载自定义配置 (不写入设备)", readConfigBin },    //
    { '3', "从设备读取配置", readConfig },                      //
    { '4', "写入配置到设备并保存", setConfig },                 //
    { '5', "写入自定义配置到文件", setConfigBin },              //
    { '6', "校验配置数据", checkConfigData },                   //
    { 'm', "调整配置", modifyConfig }                           //
};

void changeConfig() {
    if (opened) {
        showCmd(changeConfigOptions);
    } else {
        cout << "请先连接设备" << endl;
        Sleep(1000);
    }
}

void exitApp() {
    Serial.closeDevice();
    exit(0);
}

vector<option> baseOptions = {
    { '1', "连接设备", setPort },                 //
    { '2', "调整配置", changeConfig },            //
    { '3', "高级选项", deviceAdvancedFunction },  //

};

int main() {
    calcSum(&defaultConfig);
    showCmd(baseOptions);
    Serial.closeDevice();
    return 0;
}