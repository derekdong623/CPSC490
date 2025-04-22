# Compiler and flags
CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude \
	-I/Library/Frameworks/Python.framework/Versions/3.12/include/python3.12 \
	-I/Users/dd2/Library/Python/3.12/lib/python/site-packages/pybind11/include
PYBIND_CXXFLAGS := $(CXXFLAGS) -fPIC $(shell python3 -m pybind11 --includes)

# Directories
SRC_DIR := src
TEST_DIR := tests
PYBIND_DIR := bindings
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin

# Source files
MAIN_SRC := $(SRC_DIR)/main.cpp
COMMON_SRC := $(filter-out $(MAIN_SRC), $(wildcard $(SRC_DIR)/*.cpp))
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)
PYBIND_SRC := $(wildcard $(PYBIND_DIR)/*.cpp)

# Object files
COMMON_OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SRC))
TEST_OBJ := $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRC))
PYBIND_OBJ := $(patsubst $(PYBIND_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(PYBIND_SRC))
MAIN_OBJ := $(OBJ_DIR)/src_main.o

# Executables
MAIN_EXE := $(BIN_DIR)/main
TEST_EXE := $(BIN_DIR)/test_runner
PYTHON_MODULE := $(BIN_DIR)/fast_pkmn$(shell python3-config --extension-suffix)

# Default target
.PHONY: all main test pybind clean dirs
all: dirs main test

# Ensure directories exist
dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Main application
main: dirs $(MAIN_EXE)

$(MAIN_EXE): $(COMMON_OBJ) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Test executable
test: dirs $(TEST_EXE)
	$(TEST_EXE)

$(TEST_EXE): $(COMMON_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Python binding module
pybind: dirs $(PYTHON_MODULE)

$(PYTHON_MODULE): $(COMMON_OBJ) $(PYBIND_OBJ)
	$(CXX) $(PYBIND_CXXFLAGS) -dynamiclib -undefined dynamic_lookup -fPIC $^ -o $@

# Compilation rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(PYBIND_DIR)/%.cpp
	$(CXX) $(PYBIND_CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)
