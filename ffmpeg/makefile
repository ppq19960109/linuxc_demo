ifeq ($(ARCH), hisi)
CROSS_COMPILE =arm-himix100-linux-
LIBS_PARH:=hisi
DYNAMIC_LIBS= -lavcodec	-lavformat -lavutil -lpostproc -lswresample -lswscale -lx264 #-lavdevice -lavfilter 
else
CROSS_COMPILE = 
LIBS_PARH:=libs
DYNAMIC_LIBS= -lavcodec	-lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale -lx264 -lmp3lame -lxvidcore -lfdk-aac -lx265 -lrtmp 
endif

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

FFMPEG_PATH:=$(LIBS_PARH)/libffmpeg
X264_PATH:=$(LIBS_PARH)/libx264
LAME_PATH:=$(LIBS_PARH)/liblame
XVID_PATH:=$(LIBS_PARH)/libxvid
AAC_PATH:=$(LIBS_PARH)/libaac
X265_PATH:=$(LIBS_PARH)/libx265
RTMP_PATH:=$(LIBS_PARH)/librtmp

INC_FFMPEG_PATH := $(shell find $(FFMPEG_PATH)/include -maxdepth 3 -type d)

CFLAGS += $(addprefix -I , $(INC_FFMPEG_PATH))
CFLAGS += -I$(X264_PATH)/include
CFLAGS += -I$(X265_PATH)/include
CFLAGS += -I$(LAME_PATH)/include
CFLAGS += -I$(XVID_PATH)/include
CFLAGS += -I$(AAC_PATH)/include
CFLAGS += -I$(RTMP_PATH)/include
CFLAGS += -Wall

LDFLAGS += -L$(TOPDIR)
LDFLAGS += -L$(X264_PATH)/lib
LDFLAGS += -L$(X265_PATH)/lib
LDFLAGS += -L$(LAME_PATH)/lib
LDFLAGS += -L$(XVID_PATH)/lib
LDFLAGS += -L$(AAC_PATH)/lib
LDFLAGS += -L$(FFMPEG_PATH)/lib
LDFLAGS += -L$(RTMP_PATH)/lib
LDFLAGS += -L/home/ppq/libs_ffmpeg/hisi/arm-himix100-linux/arm-himix100-linux/arm-linux-uclibceabi/lib/a7_softfp_neon-vfpv4
LDFLAGS += -L/home/ppq/libs_ffmpeg/hisi/arm-himix100-linux/arm-himix100-linux/runtime_uclibc/a7_softfp_neon-vfpv4/lib
LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt $(DYNAMIC_LIBS) -lstdc++ \
		-Wl,--end-group

# SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# INC:=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
SRC := $(wildcard src/*.c)

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := app
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
