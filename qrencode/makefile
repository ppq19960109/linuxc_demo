
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)

CFLAGS += -I$(TOPDIR)$(TOPDIR)/libs/qrencode/include/
CFLAGS += -I$(SRC_INCPATH)

CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs
LDFLAGS += -L$(TOPDIR)$(TOPDIR)/libs/qrencode/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lqrencode	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := qrencode_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
