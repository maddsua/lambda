CFLAGS			=	-std=c++20
TARGETOBJ		=	core.o
OBJ_POLYFILL	=	polyfill/strings.o polyfill/date.o polyfill/mimetype.o
OBJ_HTTP		=	http/cookie.o http/headers.o http/kvcontainer.o http/url.o http/urlsearchparams.o http/method.o http/status.o
COREOBJS		=	$(OBJ_POLYFILL) $(OBJ_HTTP)

.PHONY: all all-before all-after action-custom
all: all-before $(TARGETOBJ) all-after

clean: action-custom
	rm -rf *.o *.exe *.a *.dll *.res

# target object
$(TARGETOBJ): $(COREOBJS)
	ld -relocatable $(COREOBJS) -o $(TARGETOBJ)

# polyfill stuff
polyfill/strings.o: polyfill/strings.cpp
	g++ -c polyfill/strings.cpp -o polyfill/strings.o $(CFLAGS)

polyfill/date.o: polyfill/date.cpp
	g++ -c polyfill/date.cpp -o polyfill/date.o $(CFLAGS)

polyfill/mimetype.o: polyfill/mimetype.cpp
	g++ -c polyfill/mimetype.cpp -o polyfill/mimetype.o $(CFLAGS)

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
