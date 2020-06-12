
CROSS_COMPILE =
CXX = @echo "G++ $@"; $(CROSS_COMPILE)g++
RM = rm -f
AR = ar -crv

TOPDIR = .

SRC_INCPATH = $(TOPDIR)
SRC_PATH = $(TOPDIR)

CXXFLAGS += -I$(SRC_INCPATH)
CXXFLAGS += -g -Wall -std=c++11

LDFLAGS += -L$(TOPDIR)

LIBS += -Wl,--start-group	\
		-Wl,-Bstatic -lZXingCore	\
		-Wl,-Bdynamic -ldl -lm -lpthread	\
		-Wl,--end-group

SRC += $(wildcard $(SRC_PATH)/*.cpp)


OBJ += $(SRC:%.cpp=%.o)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

LIBNAME = libCreateQrcode.a

TARGET := qrcode
.PHONY : clean all

all: $(TARGET) 

$(TARGET) : $(OBJ)
	@echo "OBJ:$(OBJ)"
	$(AR) $(LIBNAME) $(OBJ)

clean :
	$(RM) $(TARGET)
	$(RM) $(LIBNAME)
	$(RM) $(OBJ)