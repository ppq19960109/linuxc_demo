CROSS_COMPILE = 

CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -rf
AR = ar -rcs
CP = cp -r
MKDIR = mkdir -p

TOPDIR = .


CFLAGS += -Isrc
CFLAGS += -Wall -g #-lmcheck

LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  \
		-Wl,-Bdynamic -ldl -lm -lpthread -lrt \
		-Wl,--end-group

SRC := $(wildcard src/*.c)
SRC += $(wildcard src/*.cpp)

OBJ += $(filter %.o,$(SRC:%.c=%.o))
OBJ += $(filter %.o,$(SRC:%.cpp=%.o))

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := memapp
.PHONY : all clean

all: $(TARGET)

$(TARGET) : $(OBJ)
	@echo 'SRC:$(SRC)' 
	@echo 'OBJ:$(OBJ)'
	$(CC) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
