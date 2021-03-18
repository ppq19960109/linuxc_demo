BIN=libhytoolsdk.so

#CC  := mipsel-openwrt-linux-gcc
#AR  := mipsel-openwrt-linux-ar
#STRIP := mipsel-openwrt-linux-strip
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
CC  := $(CROSS_COMPILE)gcc
AR  := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

#CC  := gcc
#AR  := ar
#STRIP := strip

DIR :=./src ./src/base ./src/cjson ./src/data_repeater ./src/db ./src/dev_list ./src/epoll ./src/error ./src/hash_map ./src/link_list ./src/log ./src/misc ./src/msg_queue ./src/session ./src/timer ./src/curl
FILES :=$(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

FLAG :=-Os -MMD -Wall -I./inc -I./src/base -I./src/cjson -I./src/data_repeater -I./src/db -I./src/dev_list -I./src/epoll -I./src/error -I./src/hash_map -I./src/link_list -I./src/log -I./src/misc -I./src/msg_queue -I./src/session -I./src/timer -I./src/curl -L./lib 
LDFLAGS := -lpthread -ldl -lm -lsqlite3 -lcurl
all:
	${CC}   ${FLAG}  ${FILES}  -Wall -fPIC -shared -o $(BIN)  ${LDFLAGS}
small: 
	$(STRIP) $(BIN)
.PYTHON:clean
clean:
	-rm -rf $(BIN)  *.o