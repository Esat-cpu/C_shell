# Compiler and target executable name
CC          := gcc
TARGET      := she
VERSION     := 0.1.0


# Directories, files and their flags
BUILD_DIR   := build
SRC_DIR     := src

RELEASE_DIR  := $(BUILD_DIR)/release
DEBUG_DIR    := $(BUILD_DIR)/debug


DEBUG ?= 0

ifeq ($(DEBUG),0)
OBJ_DIR     := $(RELEASE_DIR)/obj
TARGET_EXEC := $(RELEASE_DIR)/$(TARGET)
# Flags for release
CFLAGS      := -O2
else
OBJ_DIR     := $(DEBUG_DIR)/obj
TARGET_EXEC := $(DEBUG_DIR)/$(TARGET)
# Flags for debug
CFLAGS      := -g3 -O0
endif


SRCS        := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS        := $(SRCS:%.c=$(OBJ_DIR)/%.o)

INC_DIRS    := include
INC_FLAGS   := $(addprefix -I,$(INC_DIRS))


# Test directories and files
TEST_DIR    := tests
TEST_LIB    := $(TEST_DIR)/test_lib

TEST_SRCS   := $(shell find $(TEST_DIR) -type f -name "*.c")
TEST_OBJS   := $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o)

TEST_BIN_DIR := $(BUILD_DIR)/test


# Compiler Flags
DEPS        := $(OBJS:%.o=%.d) $(TEST_OBJS:%.o=%.d)
DEPFLAGS    := -MMD -MP

CPPFLAGS    := $(INC_FLAGS) $(DEPFLAGS) -DSHE_VERSION=\"$(VERSION)\"
CFLAGS      += -Wall -Wextra
LDFLAGS     := -lreadline


# Targets

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS)


$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# Test Targets
$(OBJ_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -I$(TEST_LIB) $(CFLAGS) -c $< -o $@


$(TEST_BIN_DIR)/cd_test: $(OBJ_DIR)/$(TEST_DIR)/cd_test.o \
							$(OBJ_DIR)/$(SRC_DIR)/commands/cd.o \
							$(OBJ_DIR)/$(SRC_DIR)/shell.o
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@


$(TEST_BIN_DIR)/trim_test: $(OBJ_DIR)/$(TEST_DIR)/trim_test.o \
							$(OBJ_DIR)/$(SRC_DIR)/trim.o
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@


$(TEST_BIN_DIR)/tokenize_test: $(OBJ_DIR)/$(TEST_DIR)/tokenize_test.o \
								$(OBJ_DIR)/$(SRC_DIR)/tokenize.o \
								$(OBJ_DIR)/$(TEST_LIB)/test_lib.o
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@


$(TEST_BIN_DIR)/expansion_test: $(OBJ_DIR)/$(TEST_DIR)/expansion_test.o \
								$(OBJ_DIR)/$(TEST_LIB)/test_lib.o \
								$(OBJ_DIR)/$(SRC_DIR)/expansion.o \
								$(OBJ_DIR)/$(SRC_DIR)/tokenize.o \
								$(OBJ_DIR)/$(SRC_DIR)/shell.o
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@


test: $(TEST_BIN_DIR)/cd_test \
		$(TEST_BIN_DIR)/trim_test \
		$(TEST_BIN_DIR)/tokenize_test \
		$(TEST_BIN_DIR)/expansion_test
	$(foreach bin,$^,./$(bin);)


# Clean build
clean:
	rm -r $(BUILD_DIR)

.PHONY: all clean test

-include $(DEPS)
