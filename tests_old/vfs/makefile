
FLAGS      = -std=c++20
LIBS       = -lz -lbrotli
# lbrotli is a static libbrotli.a

BIN        = test_vfs
OBJECTS    = ../../src/filesystem.o ../../src/compress.o


.PHONY: all all-before all-after action-custom
all: all-before $(BIN) all-after

# database test
$(BIN): $(OBJECTS)
	g++ main.cpp $(OBJECTS) $(LIBS) $(FLAGS) -o $(BIN).exe

../../src/filesystem.o: ../../src/filesystem.cpp
	g++ -c ../../src/filesystem.cpp -o ../../src/filesystem.o $(LIBS) $(FLAGS)

../../src/compress.o: ../../src/compress.cpp
	g++ -c ../../src/compress.cpp -o ../../src/compress.o $(LIBS) $(FLAGS)
