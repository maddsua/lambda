
FLAGS      = -std=c++20

.PHONY: all all-before all-after base64
all: all-before all-after


base64:
	g++ -g base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)

hex:
	g++ -g hex_test.cpp ../src/hex.cpp -o hex_test.exe $(FLAGS)

generate:
	g++ -g generate_test.cpp ../src/generate.cpp ../src/hex.cpp ../src/sha1.cpp -o generate_test.exe $(FLAGS) -lwinmm

sha2:
	g++ -g sha2_test.cpp ../src/sha2.cpp -o sha2_test.exe $(FLAGS)

sha1:
	g++ -g sha1_test.cpp ../src/sha1.cpp -o sha1_test.exe $(FLAGS)