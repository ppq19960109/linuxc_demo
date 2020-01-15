
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)/include
SRC_PATH = $(TOPDIR)/src

QRCODE_SRC=$(TOPDIR)/qrcode

CFLAGS += -I$(TOPDIR)/libs/libevent/include/
CFLAGS += -I$(TOPDIR)/libs/sqlite/include/
CFLAGS += -I$(TOPDIR)/libs/qrencode/include/
CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -I$(QRCODE_SRC)
# CFLAGS += -I$(TOPDIR)/libs/sqlite3/
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)/libs/libevent/lib
LDFLAGS += -L$(TOPDIR)/libs/sqlite/lib
LDFLAGS += -L$(TOPDIR)/libs/qrencode/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -levent_core -levent  -levent_pthreads  -levent_extra -levent_openssl  -lsqlite3 -lqrencode	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)
SRC += $(wildcard $(QRCODE_SRC)/*.c)
# SRC += $(wildcard $(TOPDIR)/libs/sqlite3/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := demo
.PHONY : clean all

all: $(TARGET) 

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ 

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)