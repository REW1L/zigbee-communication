PROTOCOL_ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

SHELL = /bin/sh
CC = gcc
CPP = g++

OBJS = $(PROTOCOL_ROOT_DIR)/objs/
BIN = $(PROTOCOL_ROOT_DIR)/bin/

CPP_FLAGS = -g -c -std=c++11
C_FLAGS = -g -c
LOCAL_FLAGS =

DEVICE = RF24

ifeq ($(DEVICE), RF24)
LOCAL_FLAGS += -DRF24
DEV = -I./third-party/RF24Mesh RF24/device_actions.cpp -o ./objs/device_actions.o
else
LOCAL_FLAGS += -DZIGBEE
DEV = ./zigbee/serial_transactions.c -o ./objs/device_actions.o
endif

device:
	$(CPP) $(CPP_FLAGS) $(DEV)

PC:
	$(CC) $(C_FLAGS) $(LOCAL_FLAGS) -I./third-party/lz4 ./protocol/protocol_encode.c -o ./objs/PC_protocol_encode.o
	$(CC) $(C_FLAGS) $(LOCAL_FLAGS) ./protocol/parser.c -o ./objs/PC_parser.o
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -pthread ./protocol/Reader.cpp -o ./objs/PC_Reader.o
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) ./protocol/Sender.cpp -o ./objs/PC_Sender.o
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -pthread -I./protocol/ ./protocol/WorkerThread.cpp -o ./objs/PC_WorkerThread.o
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -I./protocol/ ./protocol/ProtocolListener.cpp -o ./objs/PC_ProtocolListener.o
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -I./protocol/ ./protocol/UserListener.cpp -o ./objs/PC_UserListener.o

$(OBJS):
	mkdir $(OBJS)

$(BIN):
	mkdir $(BIN)

lz4: $(OBJS)
	$(CC) ./third-party/lz4/lz4.c -c -o $(OBJS)/lz4.o

all: $(OBJS) $(BIN) lz4 device PC
	$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) -I./protocol/ sampleapp/sampleapp.cpp -o ./objs/SA_sampleapp.o
	$(CPP) -g -lrf24-bcm -lrf24network -lrf24mesh -pthread -Wall $(LOCAL_FLAGS) ./objs/device_actions.o ./objs/SA_*.o ./objs/PC_*.o ./objs/lz4.o -o ./bin/sampleapp
	@echo "Got it!"

unittests: $(BIN) $(OBJS) lz4
	gcc -g -c -std=c11 -I./protocol/ -I./third-party/lz4 ./tests/protocol_tests.c -o $(OBJS)/protocol_tests.o
	gcc -g -std=c11 $(OBJS)/lz4.o $(OBJS)/protocol_tests.o -o ./bin/tests
	@echo "\n\nRunning unit tests:"
	time ./bin/tests

clean:
	rm -rf $(BIN)
	rm -rf $(OBJS)
