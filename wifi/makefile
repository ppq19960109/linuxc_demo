
CROSS_COMPILE = arm-rockchip-linux-gnueabihf-
# CROSS_COMPILE = armv8l-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)
ROCK_LD_PATH = /home/ppq/host


CFLAGS += -I$(SRC_INCPATH) 
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic 	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lDeviceIo -lasound \
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

 OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := wifi
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS)  $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
