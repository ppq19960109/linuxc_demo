BASE = ../..

BIN=hysdk_demo
OBJPATH=obj
SRCPATH=src
INCPATH=inc
LIBPATH=lib
EXEPATH = .

INC = -I$(INCPATH)
LIB = -L $(LIBPATH)
#CC=gcc
#CC= mipsel-openwrt-linux-gcc
CC=/opt/OpenWrt-Toolchain-ramips-mt7688_gcc-5.3.0_musl-1.1.14.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-5.3.0_musl-1.1.14/bin/mipsel-openwrt-linux-gcc
GDB=-g
CFLAGS=-Os -MMD -Wall
LIBFLAGS = -lpthread -lm -ldl -lhytoolsdk -lsqlite3
#STRIP = strip
#STRIP = mipsel-openwrt-linux-strip
STRIP := /opt/OpenWrt-Toolchain-ramips-mt7688_gcc-5.3.0_musl-1.1.14.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-5.3.0_musl-1.1.14/bin/mipsel-openwrt-linux-strip

SRCS=$(subst $(SRCPATH)/,,$(wildcard $(SRCPATH)/*.c))
OBJS=$(addprefix $(OBJPATH)/,$(SRCS:.c=.o))
DEPS=$(patsubst %o,%d,$(OBJS))

all: $(EXEPATH)/$(BIN)

$(EXEPATH)/$(BIN): $(OBJPATH) $(OBJS)
	$(CC) $(CFLAGS) $(GDB) -Bdynamic -o $@ $(OBJS) $(LIB) $(LIBFLAGS)

$(OBJPATH)/%.o: $(SRCPATH)/%.c
	$(CC) $(CFLAGS) $(GDB) $(INC) -fPIC -o $@ -c $<

small: 
	$(STRIP) $(EXEPATH)/$(BIN)

$(OBJPATH):
	mkdir -p $(OBJPATH)

-include $(DEPS)

.PHONY: all clean


clean:
	rm -rf $(OBJS) $(DEPS) $(BIN) $(OBJPATH)