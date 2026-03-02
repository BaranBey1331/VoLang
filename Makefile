# Compilers
CC = gcc
CXX = g++

# GCC'ye "H files" klasöründe başlık dosyalarını (header) aramasını söylüyoruz (Tırnak işaretleri boşluk hatasını önler)
CFLAGS = -Wall -Wextra -O3 -std=c11 -I"src/H files" -I"optimizer"
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -I"src/H files" -I"optimizer"

# Make sistemine "C files" klasöründeki kaynak kodlarını aramasını söylüyoruz (Ters bölü işareti boşluğu kaçırır)
VPATH = src/C\ files optimizer

# Boşluklu klasörlerdeki tüm C ve C++ dosyalarını bul (Shell komutu yardımıyla)
C_SRCS = $(shell find "src/C files" optimizer -name '*.c' 2>/dev/null)
CXX_SRCS = $(shell find "src/C files" -name '*.cpp' 2>/dev/null)

# Make'in kafasının karışmaması için yolları silip sadece dosya isimlerini alıyoruz (main.c, lexer.c vb.)
C_FILES = $(notdir $(C_SRCS))
CXX_FILES = $(notdir $(CXX_SRCS))

# Obje dosyalarının obj/ klasöründe oluşmasını sağlıyoruz
OBJS = $(addprefix obj/, $(C_FILES:.c=.o) $(CXX_FILES:.cpp=.o))

TARGET = volang

# Varsayılan derleme komutu
all: $(TARGET)

# Tüm objeleri birbirine bağla
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

# C dosyalarını derle ("$<" etrafındaki tırnaklar boşluklu klasör yollarını korur)
obj/%.o: %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c "$<" -o "$@"

# C++ dosyalarını derle
obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

# Temizlik komutu
clean:
	rm -rf obj $(TARGET)

.PHONY: all clean
