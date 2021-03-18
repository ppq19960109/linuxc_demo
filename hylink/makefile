ifeq ($(VER), 0)
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-

else
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
SQLITE_PATH:=libsqlite3_u86
CFLAGS += -I$(SQLITE_PATH)/include
LDFLAGS += -L$(SQLITE_PATH)/lib
endif
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

HYTOOL_PATH:=libhytool

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(HYTOOL_PATH)/include
CFLAGS += -I$(HYTOOL_PATH)/include/base64
CFLAGS += -I$(HYTOOL_PATH)/include/cJSON
CFLAGS += -I$(HYTOOL_PATH)/include/klib
CFLAGS += -I$(HYTOOL_PATH)/include/tcp
CFLAGS += -I$(HYTOOL_PATH)/include/timer
CFLAGS += -I$(HYTOOL_PATH)/include/signal
CFLAGS += -Wall

LDFLAGS += -L$(HYTOOL_PATH)/lib
LDFLAGS += -L$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lhytool\
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt -lsqlite3	-lhytoolsdk \
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

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
