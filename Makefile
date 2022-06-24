LIBAVOID = /Users/spoenemann/OpenSource/elk/adaptagrams/cola

BIN = libavoid-server
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)

COPTS = 
INCS = -I$(LIBAVOID) -Iinclude

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SRCS))))


default:
	@echo "Use one of the targets: macos, linux"


# Compiling all libavoid-server sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	mkdir -p $(@D)
	$(CC) $(COPTS) $(INCS) -c -o $@ $<  

.SECONDEXPANSION:
	
# Linking all the stuff
$(BIN_DIR)/%: $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LOPTS) $(LIBS) -o $@ $(OBJS) $(LIBAVOID)/libavoid/.libs/libavoid.a


# Run this target on a MacOS machine
macos: CC = g++
macos: LOPTS = -mmacosx-version-min=12.4
macos: LIBS =
macos: $(BIN_DIR)/$$@/$(BIN) $?

# Run this target on a Linux machine
linux: CC = g++
linux: LOPTS =
linux: LIBS = -lpthread
linux: $(BIN_DIR)/$$@/$(BIN) $?

clean: 
	rm -rf $(BIN_DIR)
