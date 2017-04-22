PROTOCOL_ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

SHELL = /bin/sh
CC = gcc
CPP = g++

OBJS = $(PROTOCOL_ROOT_DIR)/objs
BIN = $(PROTOCOL_ROOT_DIR)/bin
LIBS_DIR = $(PROTOCOL_ROOT_DIR)/libs
LOGGER_SRC = $(PROTOCOL_ROOT_DIR)/logging
PROTOCOL_SRC = $(PROTOCOL_ROOT_DIR)/protocol


CPP_FLAGS = -g -c -std=c++11
C_FLAGS = -g -c -std=c11
LOCAL_FLAGS =

SOURCES_C = $(PROTOCOL_SRC)/parser.c \
					$(PROTOCOL_SRC)/protocol_encode.c

SOURCES_CPP = $(PROTOCOL_SRC)/Reader.cpp \
					$(PROTOCOL_SRC)/Sender.cpp \
					$(PROTOCOL_SRC)/WorkerThread.cpp \
					$(PROTOCOL_SRC)/ProtocolListener.cpp \
					$(PROTOCOL_SRC)/UserListener.cpp \
					$(LOGGER_SRC)/ProtocolLogger.cpp

INCLUDES = -I./third-party/lz4 \
					-I./protocol/ \
					-I./logging/

DEVICE = RF24

ifeq ($(DEVICE), RF24)
LOCAL_FLAGS += -DRF24
DEV = -I./third-party/RF24Mesh -I./logging/ $(PROTOCOL_ROOT_DIR)/RF24/device_actions.cpp -o $(OBJS)/device_actions.o
INCLUDES += -I./RF24/ \
						-I./third-party/RF24/ \
						-I./third-party/RF24Network/ \
						-I./third-party/RF24Mesh/

LIBS = $(LIBS_DIR)/librf24.so.1.2.0 \
				$(LIBS_DIR)/librf24network.so.1.0 \
				$(LIBS_DIR)/librf24mesh.so.1.0

# TODO: move shell script to makefile


libs:
	@echo "In case of error try to reinstall libraries"
else
LOCAL_FLAGS += -DZIGBEE
DEV = $(PROTOCOL_ROOT_DIR)/zigbee/serial_transactions.c -o $(OBJS)/device_actions.o
INCLUDES += -I./zigbee/
LIBS=
libs: $(LIBS_DIR)
endif

OBJECTS_C:=$(SOURCES_C:$(PROTOCOL_SRC)/%.c=$(OBJS)/PC_%.o)

OBJECTS_CPP:=$(SOURCES_CPP:$(PROTOCOL_SRC)/%.cpp=$(OBJS)/PC_%.o)

OBJECTS:=$(OBJECTS_C) $(OBJECTS_CPP)

$(OBJECTS_C):$(OBJS)/PC_%.o:$(PROTOCOL_SRC)/%.c
	@$(CC) $(C_FLAGS) $(LOCAL_FLAGS) $(INCLUDES) $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJECTS_CPP):$(OBJS)/PC_%.o:$(PROTOCOL_SRC)/%.cpp
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) $(INCLUDES) $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJECTS): $(OBJS) lz4 $(OBJECTS_C) $(OBJECTS_CPP)

pc: $(OBJECTS)

$(LIBS_DIR):
	mkdir -p $(LIBS_DIR)

$(OBJS):
	mkdir -p $(OBJS)

$(BIN):
	mkdir -p $(BIN)

lz4: $(OBJS)
	$(CC) $(PROTOCOL_ROOT_DIR)/third-party/lz4/lz4.c -c -o $(OBJS)/lz4.o

device:
	$(CPP) $(CPP_FLAGS) $(DEV)

sampleapp: $(BIN) pc device libs
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -I./protocol/ $(PROTOCOL_ROOT_DIR)/sampleapp/sampleapp.cpp -o $(OBJS)/SA_sampleapp.o
	$(CPP) -g -lrf24-bcm -lrf24network -std=c++11 -lrf24mesh -pthread -Wall $(LOCAL_FLAGS) $(OBJS)/device_actions.o $(OBJS)/SA_*.o $(OBJECTS) $(OBJS)/lz4.o -o $(BIN)/sampleapp
	@echo "Got sampleapp"

# all: sampleapp
	# @echo "Got it!"

unittests: $(BIN) $(OBJS) lz4
	gcc -g -c -std=c11 -I./protocol/ -I./third-party/lz4 ./tests/protocol_tests.c -o $(OBJS)/protocol_tests.o
	gcc -g -std=c11 $(OBJS)/lz4.o $(OBJS)/protocol_tests.o -o ./bin/tests
	@echo "\n\nRunning unit tests:"
	time ./bin/tests

clean:
	rm -rf $(BIN)
	rm -rf $(OBJS)
	rm -rf $(LIBS_DIR)
