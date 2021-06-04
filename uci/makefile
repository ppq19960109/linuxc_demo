CROSS_COMPILE =  /home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 2 -type d)

# LIBUCI=libuci
# LIBUBOX=libubox
CFLAGS += $(addprefix -I , $(SRC_DIRS))
# CFLAGS += -I$(LIBUCI)/include
# CFLAGS += -I$(LIBUBOX)/include/libubox
CFLAGS += -Wall #-g -lmcheck

LDFLAGS += -L$(TOPDIR)
# LDFLAGS += -L$(LIBUCI)/lib
# LDFLAGS += -L$(LIBUBOX)/lib
#-luci -lubox
LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

# SRC := $(wildcard src/*.c)
SRC += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SRC += $(wildcard src/*.cpp)

OBJ += $(filter %.o,$(SRC:%.c=%.o))
OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := uciapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo 'SRC:$(SRC)' 
	@echo 'OBJ:$(OBJ)'
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
