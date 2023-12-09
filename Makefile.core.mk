CORE_OBJ_TARGET	=	core/core.o
CORE_OBJ_DEPS	=	$(OBJ_POLYFILL) $(OBJ_HTTP) $(OBJ_ENCODING)

OBJ_POLYFILL	=	core/polyfill/strings.o core/polyfill/date.o core/polyfill/mimetype.o
OBJ_HTTP		=	core/http/cookie.o core/http/headers.o core/http/kvcontainer.o core/http/url.o core/http/urlsearchparams.o core/http/method.o core/http/status.o
OBJ_ENCODING	=	core/encoding/base64.o core/encoding/hex.o core/encoding/url.o

# target object
octo.core: $(CORE_OBJ_TARGET)

$(CORE_OBJ_TARGET): $(CORE_OBJ_DEPS)
	ld -relocatable $(CORE_OBJ_DEPS) -o $(CORE_OBJ_TARGET)

# polyfill stuff
core/polyfill/strings.o: core/polyfill/strings.cpp
	g++ -c core/polyfill/strings.cpp -o core/polyfill/strings.o $(CFLAGS)

core/polyfill/date.o: core/polyfill/date.cpp
	g++ -c core/polyfill/date.cpp -o core/polyfill/date.o $(CFLAGS)

core/polyfill/mimetype.o: core/polyfill/mimetype.cpp
	g++ -c core/polyfill/mimetype.cpp -o core/polyfill/mimetype.o $(CFLAGS)

# http stuff
core/http/cookie.o: core/http/cookie.cpp
	g++ -c core/http/cookie.cpp -o core/http/cookie.o $(CFLAGS)

core/http/headers.o: core/http/headers.cpp
	g++ -c core/http/headers.cpp -o core/http/headers.o $(CFLAGS)

core/http/kvcontainer.o: core/http/kvcontainer.cpp
	g++ -c core/http/kvcontainer.cpp -o core/http/kvcontainer.o $(CFLAGS)

core/http/url.o: core/http/url.cpp
	g++ -c core/http/url.cpp -o core/http/url.o $(CFLAGS)

core/http/urlsearchparams.o: core/http/urlsearchparams.cpp
	g++ -c core/http/urlsearchparams.cpp -o core/http/urlsearchparams.o $(CFLAGS)

core/http/method.o: core/http/method.cpp
	g++ -c core/http/method.cpp -o core/http/method.o $(CFLAGS)

core/http/status.o: core/http/status.cpp
	g++ -c core/http/status.cpp -o core/http/status.o $(CFLAGS)

# encoding stuff
core/encoding/base64.o: core/encoding/base64.cpp
	g++ -c core/encoding/base64.cpp -o core/encoding/base64.o $(CFLAGS)

core/encoding/hex.o: core/encoding/hex.cpp
	g++ -c core/encoding/hex.cpp -o core/encoding/hex.o $(CFLAGS)

core/encoding/url.o: core/encoding/url.cpp
	g++ -c core/encoding/url.cpp -o core/encoding/url.o $(CFLAGS)
