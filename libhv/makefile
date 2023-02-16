
CROSS_COMPILE = /home/ppq/rk3308sdk/rk3308_linux_v1.00_220210/buildroot/output/rockchip_rk3308_bs_release/host/bin/aarch64-rockchip-linux-gnu-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)

LIBHV_PATH:=arm_libhv
CFLAGS += -I$(TOPDIR)/$(LIBHV_PATH)/include/hv
CFLAGS += -I$(TOPDIR)/$(LIBHV_PATH)/include/
CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)/$(LIBHV_PATH)/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lhv	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)


%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := libhv_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
