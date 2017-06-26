LOCAL_FLAGS += -DZIGBEE
PR_INCLUDES += -I./zigbee/
LIBS=

PR_ZIGBEE_SRC := $(PROTOCOL_ROOT_DIR)/zigbee/device.cpp
PR_ZIGBEE_INCLUDES := $(PROTOCOL_ROOT_DIR)/zigbee/device.hpp \
                      $(PROTOCOL_ROOT_DIR)/zigbee/zigbee_definitions.h
                 


$(OBJS)/PC_zigbee.o: $(PR_ZIGBEE_INCLUDES) $(PR_ZIGBEE_SRC)
	@$(CPP) $(PROTOCOL_ROOT_DIR)/zigbee/device.cpp -std=gnu++11 -c -o $(OBJS)/PC_zigbee.o
	@echo "Compiled [PC_zigbee] successfully!"

protocol_device: $(OBJS)/PC_zigbee.o

libs:
	@echo "Serial libraries must be installed for working with USB as serial port"
