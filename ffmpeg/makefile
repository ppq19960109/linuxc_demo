CROSS_COMPILE = 

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .
LIBS_PARH:=libs
DYNAMIC_LIBS= -lavcodec	-lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale -lx264 

FFMPEG_PATH:=$(LIBS_PARH)/libffmpeg
X264_PATH:=$(LIBS_PARH)/libx264


CFLAGS += -I$(FFMPEG_PATH)/include
CFLAGS += -I$(X264_PATH)/include
CFLAGS += -Wall

LDFLAGS += -L$(FFMPEG_PATH)/lib
LDFLAGS += -L$(X264_PATH)/lib


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt $(DYNAMIC_LIBS) \
		-Wl,--end-group

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY : all clean

all: dec_video enc_video push_stream

dec_video : src/dec_video.o
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@
enc_video : src/enc_video.o
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@
push_stream : src/push_stream.o
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) src/*.o dec_video enc_video push_stream
