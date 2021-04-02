ifeq ($(ARCH), u86)
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
INSTALL = "install_u86"
HYTOOL_PATH:=libhytool_u86
HYTOOLSDK_PATH:=libs_u86/libhytoolsdk
SQLITE3_PATH:=libs_u86/libsqlite3
CURL_PATH:=libs_u86/libcurl
CFLAGS +=-I$(SQLITE3_PATH)/include 
CFLAGS +=-I$(CURL_PATH)/include 
LDFLAGS +=-L$(SQLITE3_PATH)/lib
LDFLAGS +=-L$(CURL_PATH)/lib
CFLAGS += -DARCH
else
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
INSTALL = "install"
HYTOOL_PATH:=libhytool
HYTOOLSDK_PATH:=libhytoolsdk
CFLAGS += -Irk3308
SRC += $(wildcard rk3308/*.c)
INC += $(wildcard rk3308/*.h)
endif

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(HYTOOL_PATH)/include
CFLAGS += -I$(HYTOOL_PATH)/include/base64
CFLAGS += -I$(HYTOOL_PATH)/include/cJSON
CFLAGS += -I$(HYTOOL_PATH)/include/klib
CFLAGS += -I$(HYTOOL_PATH)/include/tcp
CFLAGS += -I$(HYTOOL_PATH)/include/timer
CFLAGS += -I$(HYTOOL_PATH)/include/signal
CFLAGS += -Wall
CFLAGS += -I$(HYTOOLSDK_PATH)/include

LDFLAGS += -L$(HYTOOL_PATH)/lib
LDFLAGS += -L$(TOPDIR)
LDFLAGS += -L$(HYTOOLSDK_PATH)/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lhytool \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt -lsqlite3	-lcurl -lhytoolsdk \
		-Wl,--end-group

SRC += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := hylinkapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
