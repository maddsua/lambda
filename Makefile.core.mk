CORE_OBJ_MAIN		=	core/core.a
CORE_OBJ_DEPS		=	$(CORE_POLYFILL_OBJ) $(CORE_HTTP_OBJ) $(CORE_ENCODING_OBJ)

CORE_POLYFILL_OBJ	=	core/polyfill.a
CORE_POLYFILL_DEPS	=	core/polyfill/strings.o core/polyfill/date.o core/polyfill/mimetype.o

CORE_HTTP_OBJ		=	core/http.a
CORE_HTTP_DEPS		=	core/http/cookie.o core/http/headers.o core/http/kvcontainer.o core/http/url.o core/http/urlsearchparams.o core/http/method.o core/http/status.o

CORE_ENCODING_OBJ	=	core/encoding.a
CORE_ENCODING_DEPS	=	core/encoding/base64.o core/encoding/hex.o core/encoding/url.o

# target object
octo.core: $(CORE_OBJ_MAIN)

$(CORE_OBJ_MAIN): $(CORE_OBJ_DEPS)
	ar rvs $(CORE_OBJ_MAIN) $(CORE_OBJ_DEPS)

# polyfill stuff
$(CORE_POLYFILL_OBJ): $(CORE_POLYFILL_DEPS)
	ar rvs $(CORE_POLYFILL_OBJ) $(CORE_POLYFILL_DEPS)

core/polyfill/strings.o: core/polyfill/strings.cpp
	g++ -c core/polyfill/strings.cpp -o core/polyfill/strings.o $(CFLAGS)

core/polyfill/date.o: core/polyfill/date.cpp
	g++ -c core/polyfill/date.cpp -o core/polyfill/date.o $(CFLAGS)

core/polyfill/mimetype.o: core/polyfill/mimetype.cpp
	g++ -c core/polyfill/mimetype.cpp -o core/polyfill/mimetype.o $(CFLAGS)

# http stuff
$(CORE_HTTP_OBJ): $(CORE_HTTP_DEPS)
	ar rvs $(CORE_HTTP_OBJ) $(CORE_HTTP_DEPS)

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
$(CORE_ENCODING_OBJ): $(CORE_ENCODING_DEPS)
	ar rvs $(CORE_ENCODING_OBJ) $(CORE_ENCODING_DEPS)

core/encoding/base64.o: core/encoding/base64.cpp
	g++ -c core/encoding/base64.cpp -o core/encoding/base64.o $(CFLAGS)

core/encoding/hex.o: core/encoding/hex.cpp
	g++ -c core/encoding/hex.cpp -o core/encoding/hex.o $(CFLAGS)

core/encoding/url.o: core/encoding/url.cpp
	g++ -c core/encoding/url.cpp -o core/encoding/url.o $(CFLAGS)
