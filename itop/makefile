
CROSS_COMPILE = arm-none-linux-gnueabi-
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)

CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -I$(TOPDIR)/lvgl/
CFLAGS += -I$(TOPDIR)/lv_examples/
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.c)

OBJ += $(SRC:%.c=%.o)

LVGL_DIR = $(TOPDIR)
include $(TOPDIR)/lvgl/lvgl.mk
include $(TOPDIR)/lv_examples/lv_examples.mk
LVGL_OBJ += $(CSRCS:%.c=%.o)
LVGL_OBJ_DIR = $(TOPDIR)/obj

OUTPUT_OBJ := $(wildcard $(LVGL_OBJ_DIR)/*.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<


TARGET := itop
.PHONY : clean all

all: $(TARGET)


$(TARGET) :  $(OBJ) 
	make -C $(LVGL_OBJ_DIR)
	$(CC) $^ $(OUTPUT_OBJ)  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ 

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
	make -C $(LVGL_OBJ_DIR) clean