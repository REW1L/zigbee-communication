PR_INCLUDES := -I$(PROTOCOL_ROOT_DIR)/logging/ \
               -I$(PROTOCOL_ROOT_DIR)/third-party/lz4 \
               -I$(PROTOCOL_SRC)

PR_SOURCES_CPP := $(PROTOCOL_SRC)/Reader.cpp \
                  $(PROTOCOL_SRC)/Sender.cpp \
                  $(PROTOCOL_SRC)/WorkerThread.cpp \
                  $(PROTOCOL_SRC)/ProtocolListener.cpp \
                  $(PROTOCOL_SRC)/UserListener.cpp

PR_SOURCES_C := $(PROTOCOL_SRC)/protocol_encode.c \
                $(PROTOCOL_SRC)/parser.c \
                $(PROTOCOL_SRC)/common_functions.c \
                $(PROTOCOL_SRC)/crc16.c


PR_OBJECTS_CPP := $(PR_SOURCES_CPP:$(PROTOCOL_SRC)/%.cpp=$(OBJS)/PC_%.o)
PR_OBJECTS_C := $(PR_SOURCES_C:$(PROTOCOL_SRC)/%.c=$(OBJS)/PC_%.o)

pr_compress:
	@echo "Compressing is not implemented yet"

$(PR_OBJECTS_C):$(OBJS)/PC_%.o:$(PROTOCOL_SRC)/%.c
	@$(CC) $(C_FLAGS) $(LOCAL_FLAGS) $(PR_INCLUDES) $< -o $@
	@echo "Compiled ["$(notdir $(basename $@))"] successfully!"

$(PR_OBJECTS_CPP): $(OBJS)/PC_%.o:$(PROTOCOL_SRC)/%.cpp
	@$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) $(PR_INCLUDES) $< -o $@
	@echo "Compiled ["$(notdir $(basename $@))"] successfully!"

pr_objs: $(OBJS) $(PR_OBJECTS_CPP) $(PR_OBJECTS_C)

protocol_core: pr_objs


