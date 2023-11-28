CFLAGS		=	-std=c++20

.PHONY: all all-before all-after action-custom
all: all-before core.o all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

strings.o: strings.cpp
	g++ -c strings.cpp -o strings.o $(CFLAGS)

http.o: http.cpp
	g++ -c http.cpp -o http.o $(CFLAGS)

core.o: strings.o http.o
	ld -relocatable strings.o http.o -o core.o
