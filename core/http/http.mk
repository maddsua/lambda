
LIB_CORE_HTTP			=	core/http.a
LIB_CORE_HTTP_DEPS		=	core/http/request.o core/http/response.o core/http/cookies.o core/http/kvcontainer.o core/http/url.o core/http/urlsearchparams.o core/http/method.o core/http/status.o core/http/body.o

# http stuff
$(LIB_CORE_HTTP): $(LIB_CORE_HTTP_DEPS)
	ar rvs $(LIB_CORE_HTTP) $(LIB_CORE_HTTP_DEPS)

core/http/request.o: core/http/request.cpp
	g++ -c $(CFLAGS) core/http/request.cpp -o core/http/request.o

core/http/response.o: core/http/response.cpp
	g++ -c $(CFLAGS) core/http/response.cpp -o core/http/response.o

core/http/cookies.o: core/http/cookies.cpp
	g++ -c $(CFLAGS) core/http/cookies.cpp -o core/http/cookies.o

core/http/kvcontainer.o: core/http/kvcontainer.cpp
	g++ -c $(CFLAGS) core/http/kvcontainer.cpp -o core/http/kvcontainer.o

core/http/url.o: core/http/url.cpp
	g++ -c $(CFLAGS) core/http/url.cpp -o core/http/url.o

core/http/urlsearchparams.o: core/http/urlsearchparams.cpp
	g++ -c $(CFLAGS) core/http/urlsearchparams.cpp -o core/http/urlsearchparams.o

core/http/method.o: core/http/method.cpp
	g++ -c $(CFLAGS) core/http/method.cpp -o core/http/method.o

core/http/status.o: core/http/status.cpp
	g++ -c $(CFLAGS) core/http/status.cpp -o core/http/status.o

core/http/body.o: core/http/body.cpp
	g++ -c $(CFLAGS) core/http/body.cpp -o core/http/body.o
