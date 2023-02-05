
FLAGS      = -std=c++20

.PHONY: all all-before all-after base64
all: all-before all-after


base64:
	g++ -g base64_test.cpp ../src/base64.cpp -o base64_test.exe $(FLAGS)

util:
	g++ -g util_test.cpp ../src/util.cpp ../src/sha.cpp -o util_test.exe $(FLAGS) -lwinmm

sha:
	g++ -g sha_test.cpp ../src/sha.cpp -o sha_test.exe $(FLAGS)