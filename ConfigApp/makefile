# makefile
#
# ==============================================
# 特别注意这里缩进必须用制表符Tab 不可以用四个空格!
# ==============================================

################################################

# makefile变量的定义
# 变量名 = 变量值

# 指定编译器
CXX = g++


# 需要编译的cpp文件
SRCS += .\src\ConfigApp.cpp
SRCS += .\src\serialib.cpp

INC += -I ./include/
INC += -I ./include/share/


# 指定编译参数
#     -m32 可以使用32位库
#     -L .\lib 指定lib文件位置
#     -I .\include 指定头文件位置
#     -l xxx 指定要参与链接的 xxx.lib
#     -mwindows 指定这是Windows程序，且不会显示控制台
#     -std=c++14 指定C++标准版本
#     -finput-charset=utf-8 -fexec-charset=gbk 把文件里面的utf8编码转换为gbk用于显示
CXXFLAGS = -L .\lib $(INC) -luser32 -lgdi32 -lkernel32 -std=c++14 -static -finput-charset=utf-8 -fexec-charset=gbk
# CXXFLAGS += -mwindows

 
# Object files (replace .cpp with .o)
OBJS = $(SRCS:.cpp=.o)
 
# 最终目标程序名
TARGET = .\bin\ConfigApp.exe


################################################
# 这部分是编译对象的命令格式
# 目标 : 依赖
#     命令
# 它会先检查依赖是否完成，如果完成则执行命令，否则先执行依赖

# 默认编译就是你，直接用命令 make 输出的东西
all: $(TARGET)

# 使用命令 make run 可以运行输出的程序
run: clean all
	$(TARGET)

# 把object文件链接成可执行
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $(TARGET)

# 编译源码
%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) -c $< $(CXXFLAGS) -o $@

# Clean rule to remove object files and executable
clean:
	del $(OBJS) $(TARGET)

# Phony targets to avoid conflicts with file names
.PHONY: all clean