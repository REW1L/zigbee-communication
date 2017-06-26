PR_INCLUDES +=../RF24/
LOCAL_FLAGS += -DRF24
COMPILE_FLAGS += -lrf24-bcm -lrf24network -lrf24mesh 
DEV = 
PR_DEV_INCLUDES += -I$(PROTOCOL_ROOT_DIR)/RF24/ \
            -I$(PROTOCOL_ROOT_DIR)/third-party/RF24/ \
            -I$(PROTOCOL_ROOT_DIR)/third-party/RF24Network/ \
            -I$(PROTOCOL_ROOT_DIR)/third-party/RF24Mesh/ \
            -I$(PROTOCOL_ROOT_DIR)/logging/


LIBS = $(LIBS_DIR)/librf24.so.1.2.0 \
        $(LIBS_DIR)/librf24network.so.1.0 \
        $(LIBS_DIR)/librf24mesh.so.1.0

# TODO: move shell script to makefile

$(OBJS)/PC_RF24.o: $(PROTOCOL_ROOT_DIR)/RF24/device_actions.cpp $(PROTOCOL_ROOT_DIR)/RF24/device.hpp
	@$(CPP) -c -std=gnu++11 $(PR_DEV_INCLUDES) $(PROTOCOL_ROOT_DIR)/RF24/device_actions.cpp -o $(OBJS)/PC_RF24.o
	@echo "Compiled [PC_RF24] successfully!"

protocol_device: $(OBJS)/PC_RF24.o
