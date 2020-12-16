
CROSS_COMPILE =/home/ppq/arm32-rk3308/bin/arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

BASE64_PATH:=libs/base64
CJSON_PATH:=libs/cJSON
KLIB_PATH:=libs/klib

TUYALIB_PATH:=libs/tuyalib/lib
INC_TUYALIB_PATH:=libs/tuyalib/include

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(BASE64_PATH)
CFLAGS += -I$(CJSON_PATH)
CFLAGS += -I$(KLIB_PATH)
CFLAGS += -I$(INC_TUYALIB_PATH)
CFLAGS += -Wall

LDFLAGS += -L$(TOPDIR)
LDFLAGS += -L$(TUYALIB_PATH)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt -ltuya_gw_ext_sdk	-lpython3.6m \
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC_BASE64 := $(wildcard $(BASE64_PATH)/*.c)
INC_BASE64 := $(wildcard $(BASE64_PATH)/*.h)
SRC_CJSON := $(wildcard $(CJSON_PATH)/*.c)
INC_CJSON := $(wildcard $(CJSON_PATH)/*.h)
SRC_KLIB := $(wildcard $(KLIB_PATH)/*.c)
INC_KLIB := $(wildcard $(KLIB_PATH)/*.h)

OBJ += $(SRC:%.c=%.o)
OBJ += $(SRC_BASE64:%.c=%.o)
OBJ += $(SRC_CJSON:%.c=%.o)
OBJ += $(SRC_KLIB:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := tyapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
