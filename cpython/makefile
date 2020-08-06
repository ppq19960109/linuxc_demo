
# CROSS_COMPILE = armv8l-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)


CFLAGS += -I$(SRC_INCPATH) 
CFLAGS += -g -Wall
CFLAGS += `python3-config --cflags` `python3-config --ldflags` -fPIC
CFLAGS += -I/usr/include/python3.8/


LDFLAGS += -L$(TOPDIR)/
LDFLAGS += -L/usr/lib/python3.8/config-3.8-x86_64-linux-gnu/



LIBS += -Wl,--start-group	\
		-Wl,-Bstatic 	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lpython3.8	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := cpy_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo "OBJ:"$(OBJ)
	$(CC) $^  $(CFLAGS)  $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
