#当前路径
PWD := .
BASE_PATH := $(PWD)/..

# CROSS = aarch64-linux-gnu-
CROSS = armv8l-linux-gnueabihf-
# CROSS = arm-linux-gnueabihf-
# CC := /opt/toolchains/mips-mti-linux-gnu/2019.09-01/bin/mips-mti-linux-gnu-gcc
# AR := /opt/toolchains/mips-mti-linux-gnu/2019.09-01/bin/mips-mti-linux-gnu-ar
CC := $(CROSS)gcc
AR := $(CROSS)ar
#头文件路径
INCLUDE_PATH := -I$(BASE_PATH)/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/network_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/config_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/profile_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/adapter/sdk_adapter/include
INCLUDE_PATH += -I$(BASE_PATH)/src
INCLUDE_PATH += -I$(BASE_PATH)/src/cloud
INCLUDE_PATH += -I$(BASE_PATH)/src/local
INCLUDE_PATH += -I$(BASE_PATH)/src/adapter
#适配源文件
SOURCE_FILES := $(wildcard $(BASE_PATH)/adapter/network_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/config_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/profile_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/adapter/sdk_adapter/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/demo/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/cloud/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/local/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/adapter/*.c)
#库文件路径
LIB_FILE_PATH := $(BASE_PATH)/lib

#库文件
LIB_FILES := -Wl,--start-group -Wl,-Bstatic  -lhilinkdevicesdk -lhilinkota -Wl,-Bdynamic -ldl -lm -lpthread -Wl,--end-group 
# LIB_FILES := -Wl,--start-group -Wl,-Bstatic  -Wl,-Bdynamic  -lpthread -lm -ldl -Wl,--end-group 
#编译出的.o文件

OBJECTS_PATH := objs
OBJECTS := $(patsubst %.c, %.o, $(SOURCE_FILES))

#编译选项

COMPILE_FLAGS := -O  $(INCLUDE_PATH) -g
LINK_FLAGS := -L$(LIB_FILE_PATH) $(LIB_FILES)

#编译开始
.PHONY: all clean
all:  hilinkdemo

#进程
hilinkdemo: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LINK_FLAGS)

#编译.o
$(OBJECTS):%.o : %.c
	$(CC) $(COMPILE_FLAGS) -c $< -o $@

#清理
clean:
	rm -f $(OBJECTS) hilinkdemo
