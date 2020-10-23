
CROSS_COMPILE =/home/ppq/arm-rk/bin/arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)

CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -Wall

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := daemon
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
