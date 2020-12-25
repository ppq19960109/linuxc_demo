
CROSS_COMPILE =/home/ppq/arm32-rk3308/bin/arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p
INSTALL = "hylink"
INSTALL_INCLUDE = "$(INSTALL)/include"
INSTALL_LIB = "$(INSTALL)/lib"


TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

CJSON_PATH:=libs/cJSON
KLIB_PATH:=libs/klib
BASE64_PATH:=libs/base64

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(CJSON_PATH)
CFLAGS += -I$(KLIB_PATH)
CFLAGS += -I$(BASE64_PATH)
CFLAGS += -Wall

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC_CJSON := $(wildcard $(CJSON_PATH)/*.c)
INC_CJSON := $(wildcard $(CJSON_PATH)/*.h)
SRC_KLIB := $(wildcard $(KLIB_PATH)/*.c)
INC_KLIB := $(wildcard $(KLIB_PATH)/*.h)
SRC_BASE64 := $(wildcard $(BASE64_PATH)/*.c)
INC_KBASE64 := $(wildcard $(BASE64_PATH)/*.h)

OBJ += $(SRC:%.c=%.o)
OBJ += $(SRC_CJSON:%.c=%.o)
OBJ += $(SRC_KLIB:%.c=%.o)
OBJ += $(SRC_BASE64:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := libhylink.a
.PHONY : all install clean distclean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(AR) $@ $^

install: $(TARGET)
	$(MKDIR) $(INSTALL_LIB) $(INSTALL_INCLUDE) $(INSTALL_INCLUDE)/cJSON $(INSTALL_INCLUDE)/klib $(INSTALL_INCLUDE)/base64
	$(CP) $(TARGET) $(INSTALL_LIB)
	$(CP) $(INC) $(INSTALL_INCLUDE)
	$(CP) $(INC_CJSON) $(INSTALL_INCLUDE)/cJSON
	$(CP) $(INC_KLIB) $(INSTALL_INCLUDE)/klib
	$(CP) $(INC_KBASE64) $(INSTALL_INCLUDE)/base64
	@echo "successed installed......."

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)

distclean :
	$(RM) $(INSTALL)
	make clean
