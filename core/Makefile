CFLAGS		=	-std=c++20
COREOBJS	=	strings.o http.o

.PHONY: all all-before all-after action-custom
all: all-before core.o all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

strings.o: strings.cpp
	g++ -c strings.cpp -o strings.o $(CFLAGS)

http.o: http.cpp
	g++ -c http.cpp -o http.o $(CFLAGS)

core.o: $(COREOBJS)
	ld -relocatable $(COREOBJS) -o core.o
