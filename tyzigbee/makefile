
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

HYTOOL_PATH:=libhytool
TUYA_PATH:=libtuya

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(HYTOOL_PATH)/include
CFLAGS += -I$(HYTOOL_PATH)/include/base64
CFLAGS += -I$(HYTOOL_PATH)/include/cJSON
CFLAGS += -I$(HYTOOL_PATH)/include/klib
CFLAGS += -I$(HYTOOL_PATH)/include/tcp
CFLAGS += -I$(HYTOOL_PATH)/include/timer
CFLAGS += -I$(HYTOOL_PATH)/include/signal
CFLAGS += -I$(TUYA_PATH)/include
CFLAGS += -Wall

LDFLAGS += -L$(HYTOOL_PATH)/lib
LDFLAGS += -L$(TUYA_PATH)/lib
LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lhytool -ltuya_gw_ext_sdk	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt -lpython3.6m \
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := tyzigbeeapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
