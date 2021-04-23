CROSS_COMPILE = arm-himix100-linux-
CFLAGS += -Ilibopenssl/include
LDFLAGS += -Llibopenssl/lib

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .
SRC_DIRS := $(shell find src -maxdepth 3 -type d)

CFLAGS += -I$(TOPDIR)
CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -Wall -DWITH_OPENSSL -DWITH_NO_C_LOCALE -DWITH_DOM #-DWITH_NONAMESPACES -DSOAP_DEBUG #-DDEBUG

LDFLAGS += -L$(TOPDIR)


LIBS += -Wl,--start-group	\
		-Wl,-Bstatic \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt -lssl -lcrypto \
		-Wl,--end-group

SRC := $(wildcard *.c)
SRC += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# SRC += $(wildcard *.cpp)

OBJ := $(filter %.o,$(SRC:%.c=%.o))
# OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# %.o : %.cpp
# 	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := onvifapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(filter-out soapC.o,$(OBJ))

distclean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
