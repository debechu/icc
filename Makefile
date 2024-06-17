export BUILD_DIR  := $(abspath ./build)
export TARGET_DIR := $(abspath ./bin)
TARGET_DEPS_DIR   := $(TARGET_DIR)/deps

TARGET_NAME = icc
TARGET_EXE_FILE = $(TARGET_DIR)/$(TARGET_NAME)
TARGET_LIB_FILE = $(TARGET_DIR)/lib$(TARGET_NAME).a

INCL_DIRS   = ./include $(SAIL_INCLUDE_DIR)
INCL_FLAGS := $(addprefix -I,$(INCL_DIRS))

CLI_SRC_DIR     = ./cli
CLI_SRC_FILES  := $(shell find "$(CLI_SRC_DIR)" -name '*.c')
CLI_OBJ_FILES  := $(CLI_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
CLI_DEPS_FILES := $(CLI_OBJ_FILES:%.o=$(BUILD_DIR)/%.d)

CLI_LIBS  = sail sail-common
CLI_LIBS_FLAGS := $(addprefix -l,$(CLI_LIBS)) -L'$(TARGET_DIR)' -licc

LIB_SRC_DIR     = ./lib
LIB_SRC_FILES  := $(shell find "$(LIB_SRC_DIR)" -name '*.c')
LIB_OBJ_FILES  := $(LIB_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
LIB_DEPS_FILES := $(LIB_OBJ_FILES:%.o=$(BUILD_DIR)/%.d)

.DELETE_ON_ERROR:
.PHONY: all cli lib
all: lib cli
cli: $(TARGET_EXE_FILE)
lib: $(TARGET_LIB_FILE)

$(TARGET_EXE_FILE): ${TARGET_LIB_FILE} $(CLI_OBJ_FILES)
	@mkdir -p $(dir $@)
	gcc -o $@ $(CLI_OBJ_FILES) $(CLI_LIBS_FLAGS)

$(TARGET_LIB_FILE): $(LIB_OBJ_FILES)
	@mkdir -p $(dir $@)
	ar rcs $@ $(LIB_OBJ_FILES) $(LIB_LIBS_FILES)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc -MMD -MP $(INCL_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -r $(BUILD_DIR)
	-rm -r $(TARGET_DIR)
	-rm -r $(TARGET_DEPS_DIR)

-include $(CLI_DEPS_FILES) $(LIB_DEPS_FILES)
