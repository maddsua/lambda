
FLAGS      = -std=c++20

.PHONY: all all-before all-after base64
all: all-before all-after


base64:
	g++ base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)

hex:
	g++ hex_test.cpp ../src/hex.cpp -o hex_test.exe $(FLAGS)
