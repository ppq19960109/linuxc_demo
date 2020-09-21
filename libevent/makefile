
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)/include
SRC_PATH = src


CFLAGS += -I$(TOPDIR)$(TOPDIR)/libs/libevent/include/
CFLAGS += -I$(TOPDIR)$(TOPDIR)/libs/sqlite/include/
CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -I$(QRCODE_SRC)
# CFLAGS += -I$(TOPDIR)/libs/sqlite3/
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs/libevent/lib
LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs/sqlite/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -levent_core -levent  -levent_pthreads  -levent_extra -levent_openssl  -lsqlite3 	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)
# SRC += $(wildcard $(TOPDIR)/libs/sqlite3/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := libevent_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	echo $(SRC)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
