CROSS_COMPILE = 

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

LIVE555_PATH:=liblive555

CFLAGS += -I$(LIVE555_PATH)/include/BasicUsageEnvironment
CFLAGS += -I$(LIVE555_PATH)/include/groupsock 
CFLAGS += -I$(LIVE555_PATH)/include/liveMedia 
CFLAGS += -I$(LIVE555_PATH)/include/UsageEnvironment 
CFLAGS += -I$src
CFLAGS += -Wall

LDFLAGS += -L$(LIVE555_PATH)/lib
LDFLAGS += -L$(TOPDIR)

LIVE555_LIBS= -lBasicUsageEnvironment -lgroupsock -lliveMedia -lUsageEnvironment

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic $(LIVE555_LIBS) \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

# SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC := $(wildcard src/*.c)
SRC += $(wildcard src/*.cpp)

OBJ += $(filter %.o,$(SRC:%.c=%.o))
OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := liveapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo 'SRC:$(SRC)' 
	@echo 'OBJ:$(OBJ)'
	$(CXX) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
