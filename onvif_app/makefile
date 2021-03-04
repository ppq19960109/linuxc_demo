
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .
OPENSSL_PATH:=/home/ppq/openssl-1.0.1u/_install

SRC_DIRS := $(shell find src -maxdepth 3 -type d)

CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(OPENSSL_PATH)/include
CFLAGS += -I$(TOPDIR) 
CFLAGS += -I$(TOPDIR)/soap -I/home/ppq/gsoap-2.8/gsoap
CFLAGS += -Wall


LDFLAGS += -L$(OPENSSL_PATH)/lib
LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt	\
		-Wl,--end-group

SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SRC += $(wildcard *.c)
SRC += $(wildcard soap/*.c)

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := onvifapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
