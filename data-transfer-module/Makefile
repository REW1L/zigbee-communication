PROTOCOL_ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

SHELL = /bin/sh
CC = gcc
CPP = g++

OBJS ?= $(PROTOCOL_ROOT_DIR)/objs
BIN ?= $(PROTOCOL_ROOT_DIR)/bin
LIBS_DIR ?= $(PROTOCOL_ROOT_DIR)/libs
PR_LOGGER_SRC = $(PROTOCOL_ROOT_DIR)/logging
PROTOCOL_SRC = $(PROTOCOL_ROOT_DIR)/protocol
STATIC_LIB_DIR ?= $(PROTOCOL_ROOT_DIR)/staticlib
SHARED_LIB_DIR ?= $(PROTOCOL_ROOT_DIR)/sharedlib


CPP_FLAGS = -g -c -std=gnu++11 -Wall
C_FLAGS = -g -c -std=gnu11 -Wall
LOCAL_FLAGS =
COMPILE_FLAGS =
SHARED_LIBS_DIRS=
STATIC_LIBS_DIRS=
SHARED_LIBS=
STATIC_LIBS=
OBJECTS=


DEVICE ?= RF24


ifeq ($(DEVICE), RF24)
include $(PROTOCOL_ROOT_DIR)/RF24/RF24.mk
else
include $(PROTOCOL_ROOT_DIR)/zigbee/zigbee.mk
endif

ifeq ($(LINKTYPE), STATIC)
LIBTYPE=staticlib
STATIC_LIBS_DIRS+=$(STATIC_LIB_DIR)
STATIC_LIBS+=$(STATIC_LIB_DIR)/libconnection.a
else ifeq ($(LINKTYPE), SHARED)
LIBTYPE=sharedlib
SHARED_LIBS_DIRS+=-L$(SHARED_LIB_DIR)
SHARED_LIBS+=-lconnection
else
LIBTYPE=withoutlib
OBJECTS+=$(OBJS)/PC_*.o $(OBJS)/lz4.o
endif

include $(PROTOCOL_SRC)/protocol-core.mk
include $(PR_LOGGER_SRC)/logging.mk


$(LIBS_DIR):
	mkdir -p $(LIBS_DIR)

$(OBJS):
	mkdir -p $(OBJS)

$(BIN):
	mkdir -p $(BIN)

$(STATIC_LIB_DIR):
	mkdir -p $(STATIC_LIB_DIR)

$(SHARED_LIB_DIR):
	mkdir -p $(SHARED_LIB_DIR)

$(OBJS)/lz4.o: $(PROTOCOL_ROOT_DIR)/third-party/lz4/lz4.c
	@$(CC) -c $(PROTOCOL_ROOT_DIR)/third-party/lz4/lz4.c -o $(OBJS)/lz4.o
	@echo "Compiled [lz4] successfully!"

lz4_compress: $(OBJS)/lz4.o


$(OBJS)/SA_sampleapp.o: $(PROTOCOL_ROOT_DIR)/sampleapp/sampleapp.cpp
	@$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -I./protocol/ $(PROTOCOL_ROOT_DIR)/sampleapp/sampleapp.cpp -o $(OBJS)/SA_sampleapp.o
	@echo "Compiled [SA_sampleapp] successfully!"

sampleapp: $(BIN) $(OBJS) protocol_logger protocol_core protocol_device lz4_compress $(OBJS)/SA_sampleapp.o $(LIBTYPE)
	@echo "Creating sampleapp.."
	@$(CPP) -g $(COMPILE_FLAGS) $(LOCAL_FLAGS) -std=gnu++11 $(SHARED_LIBS_DIRS) $(SHARED_LIBS)\
	 $(STATIC_LIBS) $(OBJECTS) $(PROTOCOL_ROOT_DIR)/sampleapp/sampleapp.cpp -o $(BIN)/sampleapp
	@echo "Sampleapp linked!"

unittests: $(BIN) $(OBJS) lz4_compress
	@echo "Making executable tests..."
	@$(CC) -g -c -std=gnu11 -I./protocol/ -I./third-party/lz4 ./tests/protocol_tests.c -o $(OBJS)/PT_protocol_tests.o
	@$(CC) -g -std=gnu11 $(OBJS)/lz4.o $(OBJS)/PT_*.o -o ./bin/tests
	@echo "Compiled successfully!"
	@echo "\n\nRunning unit tests:"
	./bin/tests

clean:
	rm -rf $(BIN)
	rm -rf $(OBJS)
	rm -rf $(LIBS_DIR)
	rm -rf $(STATIC_LIB_DIR)
	rm -rf $(SHARED_LIB_DIR)

withoutlib: $(BIN) $(OBJS) $(OBJS)/SA_sampleapp.o protocol_logger protocol_core protocol_device lz4_compress

staticlib: $(STATIC_LIB_DIR) $(OBJS) protocol_logger protocol_core protocol_device lz4_compress
	@echo "Assembly of static lib.."
	@ar cr $(STATIC_LIB_DIR)/libconnection.a $(OBJS)/PC_*.o $(OBJS)/lz4.o

sharedlib: $(SHARED_LIB_DIR) $(OBJS) protocol_logger protocol_core protocol_device lz4_compress
	@echo "Assembly of shared lib.."
	@gcc -shared -g $(COMPILE_FLAGS) $(LOCAL_FLAGS) -undefined dynamic_lookup $(OBJS)/PC_*.o $(OBJS)/lz4.o -o $(SHARED_LIB_DIR)/libconnection.so
