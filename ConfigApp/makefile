CXX = g++

SRCS += .\src\ConfigApp.cpp
SRCS += .\src\serialib.cpp

INC += -I ./include/
INC += -I ./include/share/


CXXFLAGS = -L .\lib $(INC) -luser32 -lgdi32 -lkernel32 -std=c++14 -static -finput-charset=utf-8 -fexec-charset=gbk
# CXXFLAGS += -mwindows

 
OBJS = $(SRCS:.cpp=.o)
 
TARGET = .\bin\ConfigApp.exe

all: $(TARGET)

run: clean all
	$(TARGET)

mkdirs:
	if not exist "bin" mkdir -p "bin"

$(TARGET): $(OBJS) mkdirs
	$(CXX) $(OBJS) $(CXXFLAGS) -o $(TARGET)

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

clean:
	del $(OBJS) $(TARGET)

.PHONY: all clean