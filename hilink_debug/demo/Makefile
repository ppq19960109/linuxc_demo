#当前路径
PWD := .
BASE_PATH := $(PWD)/..

# CC := /opt/toolchains/mips-mti-linux-gnu/2019.09-01/bin/mips-mti-linux-gnu-gcc
# AR := /opt/toolchains/mips-mti-linux-gnu/2019.09-01/bin/mips-mti-linux-gnu-ar
CC := gcc
AR := ar
#头文件路径
INCLUDE_PATH := -I$(BASE_PATH)/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/network_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/config_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/profile_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/sdk_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/src
#适配源文件
SOURCE_FILES := $(wildcard $(BASE_PATH)/adapter/network_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/config_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/profile_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/sdk_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/demo/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/*.c)
#库文件路径
LIB_FILE_PATH := $(BASE_PATH)/lib

#库文件
# LIB_FILES := -Wl,--start-group -lhilinkdevicesdk -lhilinkota -ldl -Wl,--end-group -lpthread
LIB_FILES := -Wl,--start-group -lpthread -ldl -lm -Wl,--end-group 
#编译出的.o文件
OBJECTS_PATH := objs
OBJECTS := $(patsubst %.c, %.o, $(SOURCE_FILES))

#编译选项
COMPILE_FLAGS := -O $(INCLUDE_PATH)
LINK_FLAGS := -L$(LIB_FILE_PATH) $(LIB_FILES)

#编译开始
.PHONY: all
all: clean hilink

#清理
clean:
	rm -f $(OBJECTS) hilink

#进程
hilink: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LINK_FLAGS)

#编译.o
$(OBJECTS):%.o : %.c
	$(CC) $(COMPILE_FLAGS) -c $< -o $@
