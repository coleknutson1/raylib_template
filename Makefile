# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/raylib_hello_world

# Source files
SRCS = $(SRC_DIR)/main.cpp
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Debug build
debug: CXXFLAGS += -g -O0
debug: all

# Release build
release: CXXFLAGS += -O2
release: all

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all debug release clean run