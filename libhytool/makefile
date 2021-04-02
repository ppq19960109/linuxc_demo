ifeq ($(ARCH), u86)
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
INSTALL = "install_u86"
else
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
INSTALL = "install"
endif

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -a
MV = mv
MKDIR = mkdir -p

INSTALL_INCLUDE = "$(INSTALL)/include"
INSTALL_LIB = "$(INSTALL)/lib"

TOPDIR = .

SRC_DIRS := $(shell find tool -maxdepth 1 -type d)
TCP_DIRS := $(shell find tcp -maxdepth 1 -type d)

CJSON_PATH:=cJSON
KLIB_PATH:=klib
BASE64_PATH:=base64
SIGNAL_PATH:=signal
TIMER_PATH:=timer

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += $(addprefix -I , $(TCP_DIRS))
CFLAGS += -I$(CJSON_PATH)
CFLAGS += -I$(KLIB_PATH)
CFLAGS += -I$(BASE64_PATH)
CFLAGS += -I$(SIGNAL_PATH)
CFLAGS += -I$(TIMER_PATH)
CFLAGS += -Wall

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC_TCP:= $(foreach dir,$(TCP_DIRS),$(wildcard $(dir)/*.c))
INC_TCP:=$(foreach dir,$(TCP_DIRS),$(wildcard $(dir)/*.h))
SRC_CJSON := $(wildcard $(CJSON_PATH)/*.c)
INC_CJSON := $(wildcard $(CJSON_PATH)/*.h)
SRC_KLIB := $(wildcard $(KLIB_PATH)/*.c)
INC_KLIB := $(wildcard $(KLIB_PATH)/*.h)
SRC_BASE64 := $(wildcard $(BASE64_PATH)/*.c)
INC_BASE64 := $(wildcard $(BASE64_PATH)/*.h)
SRC_SIGNAL := $(wildcard $(SIGNAL_PATH)/*.c)
INC_SIGNAL := $(wildcard $(SIGNAL_PATH)/*.h)
SRC_TIMER := $(wildcard $(TIMER_PATH)/*.c)
INC_TIMER := $(wildcard $(TIMER_PATH)/*.h)

OBJ += $(SRC:%.c=%.o)
OBJ += $(SRC_TCP:%.c=%.o)
OBJ += $(SRC_CJSON:%.c=%.o)
OBJ += $(SRC_KLIB:%.c=%.o)
OBJ += $(SRC_BASE64:%.c=%.o)
OBJ += $(SRC_SIGNAL:%.c=%.o)
OBJ += $(SRC_TIMER:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := libhytool.a
.PHONY : all install clean distclean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(AR) $@ $^

install: $(TARGET)
	$(MKDIR) $(INSTALL_LIB) $(INSTALL_INCLUDE) $(INSTALL_INCLUDE)/tcp $(INSTALL_INCLUDE)/cJSON $(INSTALL_INCLUDE)/klib $(INSTALL_INCLUDE)/base64 $(INSTALL_INCLUDE)/signal $(INSTALL_INCLUDE)/timer
	$(CP) $(TARGET) $(INSTALL_LIB)
	$(CP) $(INC) $(INSTALL_INCLUDE)
	$(CP) $(INC_TCP) $(INSTALL_INCLUDE)/tcp
	$(CP) $(INC_CJSON) $(INSTALL_INCLUDE)/cJSON
	$(CP) $(INC_KLIB) $(INSTALL_INCLUDE)/klib
	$(CP) $(INC_BASE64) $(INSTALL_INCLUDE)/base64
	$(CP) $(INC_SIGNAL) $(INSTALL_INCLUDE)/signal
	$(CP) $(INC_TIMER) $(INSTALL_INCLUDE)/timer
	@echo "successed installed......."

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)

distclean :
	$(RM) $(INSTALL)
	make clean
