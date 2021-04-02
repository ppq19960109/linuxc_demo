BIN=libhytoolsdk.so

#CC  := mipsel-openwrt-linux-gcc
#AR  := mipsel-openwrt-linux-ar
#STRIP := mipsel-openwrt-linux-strip
ifeq ($(ARCH), u86)
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
INSTALL = "install_u86"

SQLITE3_PATH:=../libs_u86/libsqlite3
CURL_PATH:=../libs_u86/libcurl
CFLAGS +=-I$(SQLITE3_PATH)/include -L$(SQLITE3_PATH)/lib
CFLAGS +=-I$(CURL_PATH)/include -L$(CURL_PATH)/lib
else
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
INSTALL = "install"

endif

CC  := $(CROSS_COMPILE)gcc
AR  := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

#CC  := gcc
#AR  := ar
#STRIP := strip

DIR :=./src ./src/base ./src/cjson ./src/data_repeater ./src/db ./src/dev_list ./src/epoll ./src/error ./src/hash_map ./src/link_list ./src/log ./src/misc ./src/msg_queue ./src/session ./src/timer ./src/curl
FILES :=$(foreach dir,$(DIR),$(wildcard $(dir)/*.c))
FILES_INC :=$(foreach dir,$(DIR),$(wildcard $(dir)/*.h))
OBJ += $(FILES:%.c=%.o)
FLAG :=-Os -MMD -Wall -I./inc -I./src/base -I./src/cjson -I./src/data_repeater -I./src/db -I./src/dev_list -I./src/epoll -I./src/error -I./src/hash_map -I./src/link_list -I./src/log -I./src/misc -I./src/msg_queue -I./src/session -I./src/timer -I./src/curl $(CFLAGS) 
LDFLAGS := -lpthread -ldl -lm -lsqlite3 -lcurl
.PHONY : clean install distclean
%.o : %.c
	$(CC) $(FLAG) -c -o $@ $<

all:${OBJ}
	${CC}   ${FLAG}  ${FILES}  -Wall -fPIC -shared -o $(BIN)  ${LDFLAGS}
	$(CROSS_COMPILE)ar -rcs libhytoolsdk.a ${OBJ}
small: 
	$(STRIP) $(BIN)
install:
	mkdir -p install/include install/lib
	cp  -rf libhytoolsdk.a $(BIN) install/lib
	cp  -rf $(FILES_INC) install/include

clean:
	-rm -rf $(BIN)  *.o ${OBJ} libhytoolsdk.a
distclean:
	rm -rf install