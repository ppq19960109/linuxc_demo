
CROSS_COMPILE = /home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-#armv8l-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)



CFLAGS += -I$(SRC_INCPATH) 
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic 	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

# OBJ += $(SRC:%.c=%.o)
# OBJ := arp.o
OBJ := main.o

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := test
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS)   -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
