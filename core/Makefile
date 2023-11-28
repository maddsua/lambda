CFLAGS		=	-std=c++20
COREOBJS	=	http/cookie.o http/headers.o http/kvcontainer.o http/url.o http/urlsearchparams.o http/method.o

.PHONY: all all-before all-after action-custom
all: all-before core.o all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res


core.o: $(COREOBJS)
	ld -relocatable $(COREOBJS) -o core.o


http/cookie.o: http/cookie.cpp
	g++ -c http/cookie.cpp -o http/cookie.o $(CFLAGS)

http/headers.o: http/headers.cpp
	g++ -c http/headers.cpp -o http/headers.o $(CFLAGS)

http/kvcontainer.o: http/kvcontainer.cpp
	g++ -c http/kvcontainer.cpp -o http/kvcontainer.o $(CFLAGS)

http/url.o: http/url.cpp
	g++ -c http/url.cpp -o http/url.o $(CFLAGS)

http/urlsearchparams.o: http/urlsearchparams.cpp
	g++ -c http/urlsearchparams.cpp -o http/urlsearchparams.o $(CFLAGS)

http/method.o: http/method.cpp
	g++ -c http/method.cpp -o http/method.o $(CFLAGS)
