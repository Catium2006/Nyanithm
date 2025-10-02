#include <debug.h>
#include <hardware/i2c.h>
#include <i2c_config.h>
#include <i2c_port.h>
#include <pico/stdlib.h>

uint I2C_SDA[2] = { 0, 4 };
uint I2C_SCL[2] = { 1, 5 };

// 读取SDA端口
#define READ_I2CSDA(port) gpio_get(I2C_SDA[port])

// 时钟线SCL输出高电平
#define SET_I2C_SCL(port) gpio_put(I2C_SCL[port], true)
// 时钟线SCL输出低电平
#define CLR_I2C_SCL(port) gpio_put(I2C_SCL[port], false)
// 数据线SDA输出高电平
#define SET_I2C_SDA(port) gpio_put(I2C_SDA[port], true)
// 数据线SDA输出低电平
#define CLR_I2C_SDA(port) gpio_put(I2C_SDA[port], false)

void GenI2CStartSig(uint8_t port) {

    SET_I2C_SDA(port);  // 1#数据线SDA输出高电平
    SET_I2C_SCL(port);  // 2#时钟线SCL输出高电平，2-3之间的间隔须>4.7us
    sleep_us(4);        // 延时4us
    CLR_I2C_SDA(port);  // 3#数据线SDA输出低电平，3-4之间的间隔须>4.0us
    sleep_us(4);        // 延时4us
    CLR_I2C_SCL(port);  // 4#时钟线SCL输出低电平，保持I2C的时钟线SCL为低电平，准备发送或接收数据
}

void GenI2CStopSig(uint8_t port) {
    gpio_set_dir(I2C_SDA[port], true);  // 将数据线SDA设置为输出
    CLR_I2C_SCL(port);                  // 1#时钟线SCL输出低电平
    CLR_I2C_SDA(port);                  // 2#数据线SDA输出低电平
    sleep_us(4);                        // 延时4us
    SET_I2C_SCL(port);                  // 3#时钟线SCL输出高电平，3-4之间的间隔须>4.7us
    SET_I2C_SDA(port);                  // 4#数据线SDA输出高电平，发送I2C总线结束信号，4之后SDA须保持不小于4.0us的高电平
    sleep_us(4);                        // 延时4us
}

void I2CSendByte(uint8_t port, unsigned char txd) {
    unsigned char t;                    // 循环计数器
    gpio_set_dir(I2C_SDA[port], true);  // 将数据线SDA设置为输出
    CLR_I2C_SCL(port);                  // 1#时钟线SCL输出低电平,开始数据传输
    for (t = 0; t < 8; t++)             // 循环8次，从高到低取出字节的8个位
    {
        if ((txd & 0x80) >> 7)  // 2#取出字节最高位，并判断为‘0’还是‘1’，从而做出相应的操作
        {
            SET_I2C_SDA(port);  // 数据线SDA输出高电平，数据位为‘1’
        } else {
            CLR_I2C_SDA(port);  // 数据线SDA输出低电平，数据位为‘0’
        }
        txd <<= 1;          // 左移一位，次高位移到最高位
        sleep_us(2);        // 延时2us
        SET_I2C_SCL(port);  // 3#时钟线SCL输出高电平
        sleep_us(2);        // 延时2us
        CLR_I2C_SCL(port);  // 4#时钟线SCL输出低电平
        sleep_us(2);        // 延时2us
    }
}

void SendI2CNAck(uint8_t port) {
    CLR_I2C_SCL(port);                  // 1#拉低时钟线，开始传送非应答信号
    gpio_set_dir(I2C_SDA[port], true);  // 将数据线SDA设置为输出
    SET_I2C_SDA(port);                  // 2#数据线SDA输出高电平
    sleep_us(2);                        // 延时2us
    SET_I2C_SCL(port);                  // 3#时钟线SCL输出高电平，在SCL上升沿前就要把SDA拉高，为非应答信号
    sleep_us(2);                        // 延时2us
    CLR_I2C_SCL(port);                  // 4#时钟线SCL输出低电平，钳住I2C总线
}

void SendI2CAck(uint8_t port) {
    CLR_I2C_SCL(port);                  // 1#时钟线SCL输出低电平，开始传送应答信号
    gpio_set_dir(I2C_SDA[port], true);  // 将数据线SDA设置为输出
    CLR_I2C_SDA(port);                  // 2#数据线SDA输出低电平
    sleep_us(2);                        // 延时2us
    SET_I2C_SCL(port);                  // 3#时钟线SCL输出高电平,在SCL上升沿前就要把SDA拉低，为应答信号
    sleep_us(2);                        // 延时2us
    CLR_I2C_SCL(port);                  // 4#时钟线SCL输出低电平，钳住I2C总线
}


unsigned char I2CReadByte(uint8_t port, unsigned char ack) {
    unsigned char i = 0;                 // i为循环计数器
    unsigned char receive = 0;           // receive用来存放接收的数据
    gpio_set_dir(I2C_SDA[port], false);  // 1#将数据线SDA设置为输入
    for (i = 0; i < 8; i++)              // 循环8次，从高到低读取字节的8个位
    {
        CLR_I2C_SCL(port);      // 2#时钟线SCL输出低电平
        sleep_us(2);            // 延时2us
        SET_I2C_SCL(port);      // 3#时钟线SCL输出高电平
        receive <<= 1;          // 左移一位，空出新的最低位
        if (READ_I2CSDA(port))  // 4#读取数据线SDA的数据位
        {
            receive++;  // 在SCL的上升沿后，数据已经稳定，因此可以取该数据，存入最低位
        }
        sleep_us(1);  // 延时1us
    }
    if (ack == 0)  // 如果ack为NACK
    {
        SendI2CNAck(port);  // 发送NACK，无应答
    } else                  // 如果ack为ACK
    {
        SendI2CAck(port);  // 发送ACK，应答
    }
    return receive;  // 返回读取到的数据
}

unsigned char I2CWaitAck(uint8_t port) {
    unsigned char ucErrTime = 0;
    gpio_set_dir(I2C_SDA[port], true);  // 将数据线SDA设置为输出
    SET_I2C_SCL(port);                  // 时钟线SCL输出高电平
    sleep_us(1);                        // 延时1us
    while (READ_I2CSDA(port))           // 读回来的数据如果是高电平，即接收端没有应答
    {
        ucErrTime++;          // 计数器加1
        if (ucErrTime > 250)  // 如果超过250次，则判断为接收端出现故障，因此发送结束信号
        {
            GenI2CStopSig(port);  // 产生一个停止信号
            return 1;             // 返回值为1，表示没有收到应答信号
        }
    }
    CLR_I2C_SCL(port);  // 表示已收到应答信号，时钟线SCL输出低电平，钳住I2C总线
    return 0;           // 返回值为0，表示接收应答成功
}




int i2c_write(uint8_t port, uint8_t addr, uint8_t* src, size_t len, bool blocking) {
#if HWI2C
    if (port == 0)
        return i2c_write_blocking(i2c0, addr, src, len, blocking);
    if (port == 1)
        return i2c_write_blocking(i2c1, addr, src, len, blocking);
#else

#endif
}
int i2c_read(uint8_t port, uint8_t addr, uint8_t* dst, size_t len, bool blocking) {
#if HWI2C
    if (port == 0)
        return i2c_read_blocking(i2c0, addr, dst, len, blocking);
    if (port == 1)
        return i2c_read_blocking(i2c1, addr, dst, len, blocking);
#else

#endif
}

void initI2CBus(uint8_t port, uint gpio_sda, uint gpio_scl, uint baudrate) {
    I2C_SDA[port] = gpio_sda;
    I2C_SCL[port] = gpio_scl;
#if HWI2C
    int br;
    // I2C Initialisation
    if (port == 0)
        br = i2c_init(i2c0, baudrate);
    if (port == 1)
        br = i2c_init(i2c1, baudrate);
#if ENABLE_DEBUG
    printf("baudrate:%d\n", br);
#endif
    gpio_set_function(gpio_sda, GPIO_FUNC_I2C);
    gpio_set_function(gpio_scl, GPIO_FUNC_I2C);
    gpio_pull_up(gpio_sda);
    gpio_pull_up(gpio_scl);
#else
#warning Using Software I2C Interface
    gpio_init(gpio_sda);
    gpio_init(gpio_scl);
    gpio_pull_up(gpio_sda);
    gpio_pull_up(gpio_scl);
#endif
}
