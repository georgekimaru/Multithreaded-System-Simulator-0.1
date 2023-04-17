# Compiler and flags
COMPILER = g++
FLAGS = -std=c++11 -Wall -I. -pthread

# Directory names
BUILD_DIR = build
SRC_DIR = src

# Binary name
TARGET = submit

# Binary and object files
BINARY = a4w23tasks
CPP_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(addprefix $(BUILD_DIR)/,$(CPP_FILES:%.cpp=%.o))

# Files to be included in submission archive
SUBMIT_FILES = $(shell find $(SRC_DIR) \( -name '*' -o -name 'Makefile' \) -type f)


# Rules
all: setup $(BINARY)

$(BINARY): $(OBJECTS)
	$(COMPILER) $(FLAGS) $(OBJECTS) -o $(BINARY)

$(BUILD_DIR)/%.o: %.cpp
	$(COMPILER) $(FLAGS) -I$(INC_DIR) -I$(dir $<) -c $< -o $@

setup:
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)

tar:
	tar -cvf $(TARGET).tar $(SUBMIT_FILES)
	gzip $(TARGET).tar

clean:
	rm $(BUILD_DIR) -rf
	rm $(TARGET).tar.gz -f
