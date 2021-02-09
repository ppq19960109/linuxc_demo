
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

HYTOOL_PATH:=libhytool
HYLINK_PATH:=libhylink
APP_LIB_PATH:=lib
APP_INC_PATH:=include
HILINK_ADAPTER_PATH:=adapter

SRC_DIRS := $(shell find src -maxdepth 3 -type d)
HILINK_ADAPTER_PATH_DIRS := $(shell find $(HILINK_ADAPTER_PATH) -maxdepth 3 -type d)

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
CFLAGS += -I$(APP_INC_PATH)
CFLAGS += -Wall

LDFLAGS += -L$(HYTOOL_PATH)/lib
LDFLAGS += -L$(HYLINK_PATH)/lib
LDFLAGS += -L$(APP_LIB_PATH)
LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  -lhytool -lhylink -lhilinkdevicesdk -lhilinkota	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt	\
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
HILINK_ADAPTER_PATH_SRC := $(foreach dir,$(HILINK_ADAPTER_PATH_DIRS),$(wildcard $(dir)/*.c))

OBJ += $(SRC:%.c=%.o)
OBJ += $(HILINK_ADAPTER_PATH_SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := hilinkapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
