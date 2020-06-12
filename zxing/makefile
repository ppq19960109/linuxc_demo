
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -f

TOPDIR = .

SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)
ZXING_PATH = $(TOPDIR)/zxing

CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -g -Wall -std=c11

LDFLAGS += -L$(TOPDIR)
LDFLAGS += -L$(ZXING_PATH)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lZXingCore -lCreateQrcode 	\
		-Wl,-Bdynamic -ldl -lm -lpthread -lstdc++	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := qrcode
.PHONY : clean all

all: $(TARGET) 

$(TARGET) : $(OBJ)
	make -C $(ZXING_PATH)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ 

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
	make -C $(ZXING_PATH) clean