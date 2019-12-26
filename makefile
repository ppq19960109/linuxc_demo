
CROSS_COMPILE = 
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)/include
SRC_PATH = $(TOPDIR)/src

CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -I$(TOPDIR)/libs/libevent/include/
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)/libs/libevent/lib

LIBS += -Wl,-Bstatic -levent -levent_pthreads -levent_core -levent_extra  -Wl,-Bdynamic -dl -lm -lpthread   

SRC += $(wildcard $(SRC_PATH)/*.c)

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
	$(RM) $(OBJ)