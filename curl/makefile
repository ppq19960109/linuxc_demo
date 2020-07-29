
CROSS_COMPILE =
CC = @echo "GCC $@"; $(CROSS_COMPILE)gcc
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -f

TOPDIR = .
SRC_INCPATH = $(TOPDIR)/src
SRC_PATH = $(TOPDIR)/src


CFLAGS += -I$(TOPDIR)/libpc/curl/include/curl
CFLAGS += -I$(TOPDIR)/libpc/openssl/include/openssl
CFLAGS += -I$(SRC_INCPATH)
CFLAGS += -g -Wall

LDFLAGS += -L$(TOPDIR)/libpc
LDFLAGS += -L$(TOPDIR)/libpc/curl/lib
LDFLAGS += -L$(TOPDIR)/libpc/openssl/lib

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic  \
		-Wl,-Bdynamic -ldl -lm -lpthread -lcurl -lssl -lcrypto \
		-Wl,--end-group

# SRC += $(wildcard $(SRC_PATH)/*.c)
SRC += $(wildcard $(SRC_PATH)/*.cpp)

# OBJ += $(SRC:%.c=%.o)
OBJ += $(SRC:%.cpp=%.o)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

TARGET := curl_demo
.PHONY : clean all

all: $(TARGET)

$(TARGET) : $(OBJ)
	$(CXX) $^  $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJ)
