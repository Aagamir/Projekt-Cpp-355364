# Kompilator i flagi
CXX = clang++
CXXFLAGS = -std=c++20 -I./metal-cpp -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lglfw -framework Metal -framework Foundation -framework QuartzCore -framework AppKit

# Nazwa pliku wynikowego
TARGET = symulator

# Domyślny cel (wykona się po wpisaniu samego 'make')
all: $(TARGET)

# Zasada budowania programu głównego
$(TARGET): shader.metallib main.mm metal_impl.cpp
	$(CXX) $(CXXFLAGS) main.mm metal_impl.cpp $(LDFLAGS) -o $(TARGET)

# Zasada kompilacji shadera
shader.metallib: shader.metal
	xcrun -sdk macosx metal -c shader.metal -o shader.air
	xcrun -sdk macosx metallib shader.air -o shader.metallib

# Cel do szybkiego uruchamiania
run: all
	./$(TARGET)

# Cel do czyszczenia śmieci
clean:
	rm -f *.air *.metallib $(TARGET)