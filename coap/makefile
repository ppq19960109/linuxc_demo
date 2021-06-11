CROSS_COMPILE =  
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .

SRC_DIRS := $(shell find src -maxdepth 2 -type d)

LIBCOAP=libcoap
CFLAGS += $(addprefix -I , $(SRC_DIRS))
CFLAGS += -I$(LIBCOAP)/include
CFLAGS += -Wall #-g -lmcheck

LDFLAGS += -L$(TOPDIR)
LDFLAGS += -L$(LIBCOAP)/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lcoap-3 \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

# SRC := $(wildcard src/*.c)
SRC += $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
SRC += $(wildcard src/*.cpp)

OBJ += $(filter %.o,$(SRC:%.c=%.o))
OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := coapapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo 'SRC:$(SRC)' 
	@echo 'OBJ:$(OBJ)'
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
