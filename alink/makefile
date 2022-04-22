CROSS_COMPILE = arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 3 -type d)
INC_DIRS += $(shell find LinkSDK -type d \( ! -name demos \))

LINKTOOL_PATH:=liblinktool

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += $(addprefix -I , $(INC_DIRS))
CFLAGS += -I$(LINKTOOL_PATH)/include
CFLAGS += -I$(LINKTOOL_PATH)/include/base64
CFLAGS += -I$(LINKTOOL_PATH)/include/cJSON
CFLAGS += -I$(LINKTOOL_PATH)/include/klib
CFLAGS += -I$(LINKTOOL_PATH)/include/tcp
CFLAGS += -I$(LINKTOOL_PATH)/include/signal
CFLAGS += -I$(LINKTOOL_PATH)/include/timer

CFLAGS += -Wall #-Werror
ifdef DEBUG
CFLAGS += -g -lmcheck -DDEBUG
endif

LDFLAGS += -L$(LINKTOOL_PATH)/lib
LDFLAGS += -L$(TOPDIR)
LDFLAGS += -LLinkSDK

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  -lalisdk \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := linkapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CXX) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
