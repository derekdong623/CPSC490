# Compiler and flags
CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude

# Directories
SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Source files
MAIN_SRC := $(SRC_DIR)/main.cpp
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ := $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRC))
COMMON_SRC := $(filter-out $(MAIN_SRC), $(wildcard $(SRC_DIR)/*.cpp)) # Exclude main.cpp
COMMON_OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SRC))

# Executables
MAIN_EXE := $(BIN_DIR)/main
TEST_EXE := $(BIN_DIR)/test_runner

# Create necessary directories
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Build main application
$(MAIN_EXE): $(COMMON_OBJ) $(OBJ_DIR)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build test executable (does NOT use main.o)
$(TEST_EXE): $(COMMON_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main and Test targets
.PHONY: all main test clean

all: main test

main: $(MAIN_EXE)

test: $(TEST_EXE)
	$(TEST_EXE)

# Clean
clean:
	rm -rf $(BUILD_DIR)
