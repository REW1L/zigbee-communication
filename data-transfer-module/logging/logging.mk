ifeq ($(LOGGING), STD)
LOCAL_FLAGS += -DLOGGING_STDOUT
else ($(LOGGING), NONE)
LOCAL_FLAGS += -DLOGGING_NONE
endif


PR_LOGGER_INCLUDES := 

PR_LOGGER_SOURCES := $(PR_LOGGER_SRC)/ProtocolLogger.cpp

PR_LOGGER_OBJECTS := $(PR_LOGGER_SOURCES:$(PR_LOGGER_SRC)/%.cpp=$(OBJS)/PC_LOG_%.o)

$(PR_LOGGER_OBJECTS): $(OBJS)/PC_LOG_%.o : $(PR_LOGGER_SRC)/%.cpp
	@$(CPP) $(CPP_FLAGS) $(LOCAL_FLAGS) $(PR_INCLUDES) $< -o $@
	@echo "Compiled ["$(notdir $(basename $@))"] successfully!"


protocol_logger: $(OBJS) $(PR_LOGGER_OBJECTS)