
FLAGS      = -std=c++20
LIBS       = -lz -lbrotli
# lbrotli is a static libbrotli.a

.PHONY: all all-before all-after action-custom
all: all-before all-after

# http test
http: action-custom
	g++ http_test.cpp ../src/httpcore.cpp -o http_test.exe $(FLAGS)


# base64 test
base64: action-custom
	g++ base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)


# utils test
util: action-custom
	g++ util_test.cpp ../src/util.cpp ../src/sha.cpp -o util_test.exe $(FLAGS) -lwinmm


# SHA test
sha: action-custom
	g++ sha_test.cpp ../src/sha.cpp -o sha_test.exe $(FLAGS)

# SHA test
virtualfs: action-custom
	g++ virtualfs_test.cpp ../src/compression.cpp ../src/filesystem.cpp -o virtualfs_test.exe -lbrotli -lz $(FLAGS)


# database test
db: localdb_test.o ../src/localdb.o ../src/compression.o ../src/filesystem.o ../src/base64.o
	g++ localdb_test.cpp ../src/localdb.o ../src/compression.o ../src/filesystem.o ../src/base64.o $(LIBS) $(FLAGS) -o localdb_test.exe

localdb_test.o: localdb_test.cpp
	g++ -c localdb_test.cpp -o localdb_test.o $(LIBS) $(FLAGS)

../src/localdb.o: ../src/localdb.cpp
	g++ -c ../src/localdb.cpp -o ../src/localdb.o $(LIBS) $(FLAGS)

../src/compression.o: ../src/compression.cpp
	g++ -c ../src/compression.cpp -o ../src/compression.o $(LIBS) $(FLAGS)

../src/filesystem.o: ../src/filesystem.cpp
	g++ -c ../src/filesystem.cpp -o ../src/filesystem.o $(LIBS) $(FLAGS)

../src/base64.o: ../src/base64.cpp
	g++ -c ../src/base64.cpp -o ../src/base64.o $(LIBS) $(FLAGS)