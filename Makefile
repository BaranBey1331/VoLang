# Compilers
CC = gcc
CXX = g++

# Flags (Includes directories with quotes to handle spaces safely)
CFLAGS = -Wall -Wextra -O3 -std=c11 -I"src/H files" -I"optimizer"
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -I"src/H files" -I"optimizer"

TARGET = volang

# 1. Klasörlerdeki tüm .c ve .cpp dosyalarını bul
# 2. sed komutu ile her bir dosya yolunu zorla tırnak içine al ("src/C files/main.c" gibi)
# Bu sayede Make ve bash boşlukları komut sanmayacak.
C_FILES = $(shell find "src/C files" optimizer -name '*.c' 2>/dev/null | sed 's/.*/"&"/')
CXX_FILES = $(shell find "src/C files" optimizer -name '*.cpp' 2>/dev/null | sed 's/.*/"&"/')

# Default rule
all: $(TARGET)

$(TARGET):
	@echo "Derleniyor: VoLang (Unity Build Modu - Maksimum Hız)..."
	@if [ -n "$(strip $(CXX_FILES))" ]; then \
		echo "C++ dosyaları algılandı, g++ ile derleniyor..."; \
		$(CXX) $(CXXFLAGS) $(C_FILES) $(CXX_FILES) -o $(TARGET); \
	else \
		echo "Sadece C dosyaları algılandı, gcc ile derleniyor..."; \
		$(CC) $(CFLAGS) $(C_FILES) -o $(TARGET); \
	fi
	@echo "Derleme Basarili!"

# Temizlik
clean:
	rm -f $(TARGET)

.PHONY: all clean
