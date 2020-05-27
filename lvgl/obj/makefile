
CROSS_COMPILE = arm-none-linux-gnueabi-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = ..

CFLAGS += -I$(TOPDIR)
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

LVGL_DIR = $(TOPDIR)
include $(TOPDIR)/lvgl/lvgl.mk
include $(TOPDIR)/lv_examples/lv_examples.mk
LVGL_OBJ += $(CSRCS:%.c=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

TARGET := itop
.PHONY : clean all

all: $(TARGET)


$(TARGET) :  $(LVGL_OBJ)


clean :
	$(RM) $(LVGL_OBJ)