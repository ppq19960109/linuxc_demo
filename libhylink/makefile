ifeq ($(VER), 0)
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-

else
CROSS_COMPILE =/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-

endif

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -a
MV = mv
MKDIR = mkdir -p
INSTALL = "install"
INSTALL_INCLUDE = "$(INSTALL)/include"
INSTALL_LIB = "$(INSTALL)/lib"

HYLINK_PROFILE = "hyprofile"
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

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lhytool	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := libhylink.a
.PHONY : all install clean distclean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(AR) $@ $^

install: $(TARGET)
	$(MKDIR) $(INSTALL_LIB) $(INSTALL_INCLUDE)
	$(CP) $(TARGET) $(INSTALL_LIB)
	$(CP) $(INC) $(INSTALL_INCLUDE)
	$(CP) $(HYLINK_PROFILE) $(INSTALL)
	@echo "successed installed......."

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)

distclean :
	$(RM) $(INSTALL)
	make clean
