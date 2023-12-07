CFLAGS		=	-std=c++20
TARGETOBJ	=	core.o
COREOBJS	=	strings/strings.o http/cookie.o http/headers.o http/kvcontainer.o http/url.o http/urlsearchparams.o http/method.o http/status.o

.PHONY: all all-before all-after action-custom
all: all-before $(TARGETOBJ) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

# target object
$(TARGETOBJ): $(COREOBJS)
	ld -relocatable $(COREOBJS) -o $(TARGETOBJ)

# strings stuff
strings/strings.o: strings/strings.cpp
	g++ -c strings/strings.cpp -o strings/strings.o $(CFLAGS)

# http stuff
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

http/status.o: http/status.cpp
	g++ -c http/status.cpp -o http/status.o $(CFLAGS)
