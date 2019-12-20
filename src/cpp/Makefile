
include $(K273_PATH)/src/cpp/Makefile.in

INCLUDE_PATHS += -I $(MUZERO_PATH)/src/cpp

LIBS = -L $(K273_PATH)/src/cpp/k273 -lk273

CFLAGS += -fPIC

SRCS += statemachine/basestate.cpp statemachine/statemachine.cpp

HEX = games/hex
SRCS += $(HEX)/desc.cpp $(HEX)/sm.cpp $(HEX)/board.cpp

PLAYER = agents/player
SRCS += $(PLAYER)/randomplayer.cpp

SRCS += testing/perf_test.cpp

CORE_OBJS := $(SRCS:.cpp=.o)

LIB_NAME = libmuzero.so

BINS = testing/test_hex.bin

OBJS := $(CORE_OBJS) $(BINS:.bin=.o)
DEPS = $(OBJS:.o=.d)

# Top level
all: $(CORE_OBJS) $(LIB_NAME) $(BINS)

# Compiles
$(LIB_NAME): $(CORE_OBJS)
	$(CPP) -shared $(CORE_OBJS) $(LDFLAGS) $(LIBS) -pthread -lrt -o $@

%.bin: %.cpp $(CORE_OBJS)
	$(CPP) $(INCLUDE_PATHS) $(CFLAGS) $(LDFLAGS) $(CORE_OBJS) $(EXTERNAL_LIBS) $(LIBS) $< -o $@

%.o : %.cpp
	$(CPP) $(INCLUDE_PATHS) $(CFLAGS_FORLIBS) -c -o $@ $<

# Cleans
clean :
	$(RM) $(BINS) $(DEPS) $(OBJS)

-include $(DEPS)
.PHONY: all clean
