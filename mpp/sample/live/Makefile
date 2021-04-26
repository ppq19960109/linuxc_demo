# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c)
SRCS += $(wildcard *.cpp)
TARGET := venc_live

TARGET_PATH := $(PWD)

# compile linux or HuaweiLite
# include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
CXX = @echo "G++ $@"; $(CROSS)g++
LIVE555_PATH:=$(TARGET_PATH)/../liblive555

CFLAGS += -I$(LIVE555_PATH)/include/BasicUsageEnvironment
CFLAGS += -I$(LIVE555_PATH)/include/groupsock 
CFLAGS += -I$(LIVE555_PATH)/include/liveMedia 
CFLAGS += -I$(LIVE555_PATH)/include/UsageEnvironment 
CFLAGS += -Wall -DNO_OPENSSL=1
CFLAGS += -L$(LIVE555_PATH)/lib

LIVE555_LIBS= -lBasicUsageEnvironment -lgroupsock -lliveMedia -lUsageEnvironment

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<
# target source
#OBJS  := $(SRCS:%.c=%.o)
OBJS := $(filter %.o,$(SRCS:%.c=%.o))
OBJS += $(filter %.o,$(SRCS:%.cpp=%.o))

CFLAGS += $(COMM_INC)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(OBJS)
	$(CXX) $(CFLAGS) -lpthread -lm -o $(TARGET_PATH)/$@ $^ -Wl,--start-group $(MPI_LIBS) $(SENSOR_LIBS) $(AUDIO_LIBA) $(REL_LIB)/libsecurec.a $(LIVE555_LIBS) -Wl,--end-group

clean:
	@rm -f $(TARGET_PATH)/$(TARGET)
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4