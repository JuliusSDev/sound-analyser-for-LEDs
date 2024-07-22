# Compiler and flags
CC := g++
CFLAGS := -g -Wall -pedantic

# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include
LIB_DIR := /usr/lib

# Header and source files
HDR := $(wildcard $(SRC_DIR)/*.hpp)
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Libraries
LIBS := -lfftw3 -lportaudio -lbluetooth
INCLUDE_PATHS := -I$(INCLUDE_DIR) -I/usr/include
LIB_PATHS := -L$(LIB_DIR)

# Executable
EXECUTABLE := $(BUILD_DIR)/SoundAnalyser

# Phony targets
.PHONY: all clean check run

# Default target
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(SRCS) $(HDR) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_PATHS) $(LIB_PATHS) $(SRCS) $(LIBS) -o $(EXECUTABLE)

# Create the build directory if it does not exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Check the program with Valgrind
check: $(EXECUTABLE)
	valgrind --leak-check=full --track-origins=yes ./$(EXECUTABLE)

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)
