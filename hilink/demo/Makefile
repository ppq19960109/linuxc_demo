#当前路径
PWD := .
BASE_PATH := $(PWD)/..

CROSS = /home/ppq/arm32-rk3308/bin/arm-rockchip-linux-gnueabihf-
# CROSS = armv8l-linux-gnueabihf-
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
INCLUDE_PATH += -I$(BASE_PATH)/src/cJSON
INCLUDE_PATH += -I$(BASE_PATH)/src/rk3308
INCLUDE_PATH += -I$(BASE_PATH)/src/frame
INCLUDE_PATH += -I$(BASE_PATH)/lib/curl/include/curl

INCLUDE_PATH += -I$(PWD)$(PWD)/$(PWD)$(PWD)/libuv/libuv/include
INCLUDE_PATH += -I$(BASE_PATH)/lib/libevent_rk3308/include
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
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/cJSON/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/rk3308/*.c)
SOURCE_FILES += $(wildcard $(BASE_PATH)/src/frame/*.c)
#库文件路径
LIB_FILE_PATH := -L$(BASE_PATH)/lib
# LIB_FILE_PATH += -L/home/ppq/arm-rk/lib
LIB_FILE_PATH +=  -L$(PWD)$(PWD)/$(PWD)$(PWD)/libuv/libuv/lib
LIB_FILE_PATH +=  -L$(BASE_PATH)/lib/libevent_rk3308/lib
#库文件
LIB_FILES := -Wl,--start-group -Wl,-Bstatic -lhilinkdevicesdk -lhilinkota  -luv -levent_core -levent  -levent_pthreads  -levent_extra -levent_openssl -Wl,-Bdynamic -lrt -ldl -lm -lpthread  -lDeviceIo -lasound -Wl,--end-group 
# LIB_FILES := -Wl,--start-group -Wl,-Bstatic  -Wl,-Bdynamic  -lpthread -lm -ldl -Wl,--end-group 
#编译出的.o文件

OBJECTS_PATH := objs
OBJECTS := $(patsubst %.c, %.o, $(SOURCE_FILES))

#编译选项

COMPILE_FLAGS := -O  $(INCLUDE_PATH) -DUSE_LIBUV=0 -DUSE_LIBEVENT=0
LINK_FLAGS := $(LIB_FILE_PATH) $(LIB_FILES)

DAEMON_PATH=../src/daemon
#编译开始
.PHONY: all clean daemon
all:  hilinkapp daemon

#进程
hilinkapp: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LINK_FLAGS)

daemon: 
	make -C $(DAEMON_PATH)
	cp $(DAEMON_PATH)/hyDaemon .

daemon_clean:
	make -C $(DAEMON_PATH) clean
	rm -f aemon

#编译.o
$(OBJECTS):%.o : %.c
	$(CC) $(COMPILE_FLAGS) -c $< -o $@

#清理
clean: daemon_clean
	rm -f $(OBJECTS) hilinkapp hyDaemon
