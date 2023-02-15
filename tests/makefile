
FLAGS      = -std=c++20
LIBS       = -lz -lbrotli

.PHONY: all all-before all-after base64
all: all-before all-after


# base64 test
base64:
	g++ base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)


# utils test
util:
	g++ util_test.cpp ../src/util.cpp ../src/sha.cpp -o util_test.exe $(FLAGS) -lwinmm


# SHA test
sha:
	g++ sha_test.cpp ../src/sha.cpp -o sha_test.exe $(FLAGS)


# database test
db: radish_test.o ../src/radishdb.o ../src/compression.o ../src/filesystem.o ../src/base64.o
	g++ radish_test.cpp ../src/radishdb.o ../src/compression.o ../src/filesystem.o ../src/base64.o $(LIBS) $(FLAGS) -o radish_test.exe

radish_test.o: radish_test.cpp
	g++ -c radish_test.cpp -o radish_test.o $(LIBS) $(FLAGS)

../src/radishdb.o: ../src/radishdb.cpp
	g++ -c ../src/radishdb.cpp -o ../src/radishdb.o $(LIBS) $(FLAGS)

../src/compression.o: ../src/compression.cpp
	g++ -c ../src/compression.cpp -o ../src/compression.o $(LIBS) $(FLAGS)

../src/filesystem.o: ../src/filesystem.cpp
	g++ -c ../src/filesystem.cpp -o ../src/filesystem.o $(LIBS) $(FLAGS)

../src/base64.o: ../src/base64.cpp
	g++ -c ../src/base64.cpp -o ../src/base64.o $(LIBS) $(FLAGS)