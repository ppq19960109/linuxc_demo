
CROSS_COMPILE =arm-xm-linux-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)


PN512_PATH = $(TOPDIR)/pn512

CFLAGS += -I$(PN512_PATH)
CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)
SRC += $(wildcard $(PN512_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := i2c_demo
.PHONY : clean all

all: $(TARGET) 

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ 

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)