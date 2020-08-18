CXX = g++
AR = ar
CFLAGS = -g -Wall --std=c++17
LDFLAGS = 
INCLUDE = include

BIN_DIR = bin
OBJ_DIR = obj

PACK_BIN = $(BIN_DIR)/resource_pack
UNPACK_BIN = $(BIN_DIR)/resource_unpack
LOAD_BIN = $(BIN_DIR)/libresource_load.a

ifeq ($(OS),Windows_NT)
	PACK_BIN := $(PACK_BIN).exe
	UNPACK_BIN := $(UNPACK_BIN).exe
endif

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

all: $(PACK_BIN) $(UNPACK_BIN) $(LOAD_BIN)

clean:
	rm -rf bin
	rm -rf obj

$(PACK_BIN): src/resource_pack.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) -o $@ $< -lbz2

$(UNPACK_BIN): src/resource_unpack.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) -o $@ $< -lbz2

$(LOAD_BIN): $(OBJ_DIR)/resource_load.o
	$(AR) rcs $@ $<

$(OBJ_DIR)/resource_load.o: src/resource_load.cpp include/resource_load.h
	$(CXX) -c $(CFLAGS) -I$(INCLUDE) -o $@ src/resource_load.cpp
