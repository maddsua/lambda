
FLAGS      = -std=c++20

.PHONY: all all-before all-after base64
all: all-before all-after


base64:
	g++ -g base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)

hex:
	g++ -g hex_test.cpp ../src/hex.cpp -o hex_test.exe $(FLAGS)

hash:
	g++ -g roundhash_test.cpp ../src/roundhash.cpp -o roundhash_test.exe $(FLAGS)