CC = g++
AR = ar
CFLAGS = -g -Wall --std=c++11
LDFLAGS = 
INCLUDE = include

BIN_DIR = bin
OBJ_DIR = obj

PACK_BIN = $(BIN_DIR)/resource_pack
UNPACK_BIN = $(BIN_DIR)/resource_unpack
LOAD_BIN = $(BIN_DIR)/libresource_load.a

$(shell mkdir -p $(BIN_DIR) >/dev/null)
$(shell mkdir -p $(OBJ_DIR) >/dev/null)

all: $(PACK_BIN) $(UNPACK_BIN) $(LOAD_BIN)

clean:
	rm -rf bin
	rm -rf obj

$(PACK_BIN): src/resource_pack.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) -o $@ $<

$(UNPACK_BIN): src/resource_unpack.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) -o $@ $<

$(LOAD_BIN): $(OBJ_DIR)/resource_load.o
	$(AR) rcs $@ $<

$(OBJ_DIR)/resource_load.o: src/resource_load.cpp include/resource_load.h
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -o $@ src/resource_load.cpp
