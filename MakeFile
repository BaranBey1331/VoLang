# Compilers
CC = gcc
CXX = g++

# Flags (-I"src/H_files" and -I"optimizer" to find headers automatically)
CFLAGS = -Wall -Wextra -O3 -std=c11 -I"src/H_files" -I"optimizer"
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -I"src/H_files" -I"optimizer"

# Directories
SRC_DIR = src/C_files
OPT_DIR = optimizer
OBJ_DIR = obj

# Find all C and C++ files
C_SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(OPT_DIR)/*.c)
CXX_SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(filter $(SRC_DIR)/%.c, $(C_SRCS))) \
       $(patsubst $(OPT_DIR)/%.c, $(OBJ_DIR)/%.o, $(filter $(OPT_DIR)/%.c, $(C_SRCS))) \
       $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CXX_SRCS))

TARGET = volang

# Default rule
all: $(TARGET)

# Linking everything together
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

# Compiling C files from src
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling C files from optimizer
$(OBJ_DIR)/%.o: $(OPT_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling C++ files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
