# Firmware

此固件使用 `Pico SDK` 直接开发，部分芯片的驱动修改自有关 `Arduino` 库。

若要编译此固件，建议在 `Visual Studio Code` 中安装 [【树莓派Pico插件】](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico)，并导入这个项目。点击`Compile`即可编译。

编译出来的文件在 `build` 目录。

主控板子上有一个按钮，按住按钮再插数据线可以进入下载模式，电脑上会识别一个存储设备插入，把编译得到的`uf2`文件放进去即可自动下载。

# Library
special thanks to these libs below

[Adafruit_MPR121](https://github.com/adafruit/Adafruit_MPR121)

[vl53l0x-arduino](https://github.com/pololu/vl53l0x-arduino)