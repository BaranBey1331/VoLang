# Compilers
CC = gcc
CXX = g++

# Flags (Includes directories with quotes to handle spaces safely)
CFLAGS = -Wall -Wextra -O3 -std=c11 -I"src/H files" -I"optimizer"
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -I"src/H files" -I"optimizer"

TARGET = volang

# Find files and enclose paths in quotes
C_FILES = $(shell find "src/C files" optimizer -name '*.c' 2>/dev/null | sed 's/.*/"&"/')
CXX_FILES = $(shell find "src/C files" optimizer -name '*.cpp' 2>/dev/null | sed 's/.*/"&"/')

# Default rule
all: $(TARGET)

$(TARGET):
	@echo "Building VoLang (Unity Build Mode)..."
	@if [ -n "$(strip $(CXX_FILES))" ]; then \
		echo "Compiling with g++..."; \
		$(CXX) $(CXXFLAGS) $(C_FILES) $(CXX_FILES) -o $(TARGET); \
	else \
		echo "Compiling with gcc..."; \
		$(CC) $(CFLAGS) $(C_FILES) -o $(TARGET); \
	fi
	@echo "Build Successful!"

# Clean
clean:
	rm -f $(TARGET)

.PHONY: all clean
