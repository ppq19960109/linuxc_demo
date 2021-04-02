ifeq ($(ARCH), u86)
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
INSTALL = "install_u86"
HYTOOL_PATH:=libhytool_u86
HYLINK_PATH:=libhylink_u86
PAHOMQTT_PATH:=libs_u86/libpahomqttc
CFLAGS +=-I$(PAHOMQTT_PATH)/include
LDFLAGS += -L$(PAHOMQTT_PATH)/lib
else
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
INSTALL = "install"
HYTOOL_PATH:=libhytool
HYLINK_PATH:=libhylink
endif

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += $(addprefix -I , $(HILINK_ADAPTER_PATH_DIRS))
CFLAGS += -I$(HYTOOL_PATH)/include
CFLAGS += -I$(HYTOOL_PATH)/include/base64
CFLAGS += -I$(HYTOOL_PATH)/include/cJSON
CFLAGS += -I$(HYTOOL_PATH)/include/klib
CFLAGS += -I$(HYTOOL_PATH)/include/signal
CFLAGS += -I$(HYTOOL_PATH)/include/tcp
CFLAGS += -I$(HYTOOL_PATH)/include/timer
CFLAGS += -I$(HYLINK_PATH)/include
CFLAGS += -Wall

LDFLAGS += -L$(HYTOOL_PATH)/lib
LDFLAGS += -L$(HYLINK_PATH)/lib
LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  -lhytool -lhylink	-lpaho-mqtt3c \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt 	\
		-Wl,--end-group
#-lpaho-mqtt3a -lpaho-mqtt3as -lpaho-mqtt3c -lpaho-mqtt3cs
SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := mqttapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
