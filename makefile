
CROSS_COMPILE = 
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)/include
SRC_PATH = $(TOPDIR)/src

CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -I$(TOPDIR)/libs/libevent/include/
CFLAGS += -I$(TOPDIR)/libs/sqlite/include/
# CFLAGS += -I$(TOPDIR)/libs/sqlite3/
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)/libs/libevent/lib
LDFLAGS += -L$(TOPDIR)/libs/sqlite/lib

LIBS += -Wl,-Bstatic -levent_core -levent  -levent_pthreads  -levent_extra -levent_openssl  -lsqlite3  -Wl,-Bdynamic -ldl -lm -lpthread 
SRC += $(wildcard $(SRC_PATH)/*.c)
# SRC += $(wildcard $(TOPDIR)/libs/sqlite3/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := http_demo
.PHONY : clean all

all: $(TARGET) 

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ 

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ) src/*.o