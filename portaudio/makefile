
CROSS_COMPILE = #arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = src
SRC_PATH = src

CFLAGS += -I. -Ilibportaudio/include
CFLAGS += -I$(SRC_INCPATH)

CFLAGS += -g -Wall

LDFLAGS += -L. -Llibportaudio/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lportaudio	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lasound \
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := portaudio_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
