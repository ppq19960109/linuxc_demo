
CROSS_COMPILE = armv8l-linux-gnueabihf-
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
OBJ := ap.o

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
