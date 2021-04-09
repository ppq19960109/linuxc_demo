CROSS_COMPILE = 

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

EASYRTSP_PATH=libEasyRTSPClient
EASYPUSH_PATH=..
CFLAGS += -I$(EASYRTSP_PATH)/Include
CFLAGS += -I$(EASYPUSH_PATH)/Include
CFLAGS += -I$src
CFLAGS += -Wall

LDFLAGS += -L$(EASYRTSP_PATH)/Lib/x64
LDFLAGS += -L$(EASYPUSH_PATH)/Lib/x64
LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -leasyrtspclient -leasypusher \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

# SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC := $(wildcard *.c)
SRC += $(wildcard *.cpp)

OBJ += $(filter %.o,$(SRC:%.c=%.o))
OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := easyapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo 'SRC:$(SRC)' 
	@echo 'OBJ:$(OBJ)'
	$(CXX) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
