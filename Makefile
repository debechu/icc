export BUILD_DIR  := $(abspath ./build)
export TARGET_DIR := $(abspath ./bin)
TARGET_DEPS_DIR   := $(TARGET_DIR)/deps

TARGET_NAME = icc
TARGET_EXE_FILE = $(TARGET_DIR)/$(TARGET_NAME)
TARGET_LIB_FILE = $(TARGET_DIR)/lib$(TARGET_NAME).a

INCL_DIRS   = ./include ./deps/sail/src '$(BUILD_DIR)/sail/include'
INCL_FLAGS := $(addprefix -I,$(INCL_DIRS))

CLI_SRC_DIR     = ./cli
CLI_SRC_FILES  := $(shell find "$(CLI_SRC_DIR)" -name '*.c')
CLI_OBJ_FILES  := $(CLI_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
CLI_DEPS_FILES := $(CLI_OBJ_FILES:%.o=$(BUILD_DIR)/%.d)

CLI_LIBS        = sail
CLI_LIBS_FLAG  := '-L$(TARGET_DIR)' '-l:$(TARGET_LIB_FILE)' $(addprefix -l,$(CLI_LIBS))
CLI_LIBS_FILES := $(CLI_LIBS:%=$(TARGET_DEPS_DIR)/lib%.a)

LIB_SRC_DIR     = ./lib
LIB_SRC_FILES  := $(shell find "$(LIB_SRC_DIR)" -name '*.c')
LIB_OBJ_FILES  := $(LIB_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
LIB_DEPS_FILES := $(LIB_OBJ_FILES:%.o=$(BUILD_DIR)/%.d)

LIB_LIBS        = 
LIB_LIBS_FILES := $(LIB_LIBS:%=$(TARGET_DEPS_DIR)/lib%.a)

.DELETE_ON_ERROR:
.PHONY: all cli lib deps docopt
all: lib cli
cli: $(TARGET_EXE_FILE)
lib: $(TARGET_LIB_FILE)
deps: $(LIB_LIBS_FILES) $(CLI_LIBS_FILES)

$(TARGET_EXE_FILE): $(CLI_LIBS_FILES) $(CLI_OBJ_FILES)
	@mkdir -p $(dir $@)
	gcc -o $@ $(CLI_OBJ_FILES) $(CLI_LIBS_FLAGS)

$(TARGET_LIB_FILE): $(LIB_LIBS_FILES) $(LIB_OBJ_FILES)
	@mkdir -p $(dir $@)
	ar rcs $@ $(LIB_OBJ_FILES) $(LIB_LIBS_FILES)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc -MMD -MP $(INCL_FLAGS) -c $< -o $@

SAIL_BUILD_OPTIONS = \
	-D BUILD_SHARED_LIBS=OFF \
	-D BUILD_TESTING=OFF \
	-D SAIL_BUILD_APPS=OFF \
	-D SAIL_BUILD_EXAMPLES=OFF
$(TARGET_DEPS_DIR)/libsail.a: ./build/sail
	cmake --build $<
./build/sail: ./deps/sail
	cmake -S $< -B $@ $(SAIL_BUILD_OPTIONS) \
		-D CMAKE_ARCHIVE_OUTPUT_DIRECTORY='$(TARGET_DEPS_DIR)' \
		-D CMAKE_LIBRARY_OUTPUT_DIRECTORY='$(TARGET_DEPS_DIR)'
	# This file is generated for some reason.
	-rm ./file.tiff

.PHONY: clean
clean:
	-rm -r $(BUILD_DIR)
	-rm -r $(TARGET_DIR)
	-rm -r $(TARGET_DEPS_DIR)

-include $(CLI_DEPS_FILES) $(LIB_DEPS_FILES)
