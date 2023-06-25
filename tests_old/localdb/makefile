
FLAGS      = -std=c++20
LIBS       = -lz -lbrotli
# lbrotli is a static libbrotli.a

BIN        = test_localdb
OBJECTS    = ../../src/localdb.o ../../src/compress.o ../../src/util.o ../../src/sha.o


.PHONY: all all-before all-after action-custom
all: all-before $(BIN) all-after

# database test
$(BIN): $(OBJECTS)
	g++ main.cpp $(OBJECTS) $(LIBS) $(FLAGS) -o $(BIN).exe

../../src/localdb.o: ../../src/localdb.cpp
	g++ -c ../../src/localdb.cpp -o ../../src/localdb.o $(LIBS) $(FLAGS)

../../src/compress.o: ../../src/compress.cpp
	g++ -c ../../src/compress.cpp -o ../../src/compress.o $(LIBS) $(FLAGS)

../../src/util.o: ../../src/util.cpp
	g++ -c ../../src/util.cpp -o ../../src/util.o $(LIBS) $(FLAGS)

../../src/sha.o: ../../src/sha.cpp
	g++ -c ../../src/sha.cpp -o ../../src/sha.o $(LIBS) $(FLAGS)