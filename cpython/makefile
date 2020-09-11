
CROSS_COMPILE = arm-rockchip-linux-gnueabihf-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)


CFLAGS += -I$(SRC_INCPATH) 
CFLAGS += -g -Wall

# CFLAGS += `python3-config --cflags` `python3-config --ldflags` -fPIC
# CFLAGS += -I/usr/include/python3.6m/
CFLAGS += -I/home/ppq/python3.6.3/Include
CFLAGS += -I/home/ppq/python3.6.3/

LDFLAGS += -L$(TOPDIR)/
# LDFLAGS += -L/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/
LDFLAGS += -L/home/ppq/python3.6.3

#-lpython3.6m 
LIBS += -Wl,--start-group	\
		-Wl,-Bstatic 	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lpython3.6m	\
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
