
LIB_CORE				=	core/core.a
LIB_CORE_DEPS			=	$(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_HTTP_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_NETWORK_DEPS) $(LIB_CORE_COMPRESS_DEPS) $(LIB_CORE_SERVER_DEPS) $(LIB_CORE_CRYPTO_DEPS) $(LIB_CORE_HTML_DEPS) $(LIB_CORE_JSON_DEPS) $(LIB_CORE_ERROR_DEPS)

LIB_CORE_POLYFILL		=	core/polyfill.a
LIB_CORE_POLYFILL_DEPS	=	core/polyfill/strings.o core/polyfill/date.o core/polyfill/mimetype.o

LIB_CORE_HTTP			=	core/http.a
LIB_CORE_HTTP_DEPS		=	core/http/request.o core/http/response.o core/http/cookies.o core/http/kvcontainer.o core/http/url.o core/http/urlsearchparams.o core/http/method.o core/http/status.o core/http/body.o

LIB_CORE_ENCODING		=	core/encoding.a
LIB_CORE_ENCODING_DEPS	=	core/encoding/base64.o core/encoding/hex.o core/encoding/url.o

LIB_CORE_NETWORK		=	core/network.a
LIB_CORE_NETWORK_DEPS	=	core/network/tcp/connection.o core/network/tcp/listen.o

LIB_CORE_COMPRESS		=	core/compression.a
LIB_CORE_COMPRESS_DEPS	=	core/compression/streams.o core/compression/brotli.o core/compression/zlib.o

LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_DEPS	=	core/server/server.o core/server/http/transport.o core/server/http/queue.o core/server/console.o core/server/handlers/serverless.o core/server/handlers/connection.o

LIB_CORE_HTML			=	core/html.a
LIB_CORE_HTML_TEMPLATES	=	core/html/resources/servicepage.res
LIB_CORE_HTML_DEPS		=	core/html/templates.o core/html/engine.o $(LIB_CORE_HTML_TEMPLATES)

LIB_CORE_JSON			=	core/json.a
LIB_CORE_JSON_DEPS		=	core/json/property.o core/json/parse.o core/json/stringify.o

LIB_CORE_CRYPTO			=	core/crypto.a
LIB_CORE_CRYPTO_DEPS	=	core/crypto/sha1.o  core/crypto/uid.o

LIB_CORE_ERROR			=	core/error.a
LIB_CORE_ERROR_DEPS		=	core/error/apierror.o


# target object
lambda.core: $(LIB_CORE)

$(LIB_CORE): $(LIB_CORE_DEPS)
	ar rvs $(LIB_CORE) $(LIB_CORE_DEPS)


# polyfill stuff
$(LIB_CORE_POLYFILL): $(LIB_CORE_POLYFILL_DEPS)
	ar rvs $(LIB_CORE_POLYFILL) $(LIB_CORE_POLYFILL_DEPS)

core/polyfill/strings.o: core/polyfill/strings.cpp
	g++ -c $(CFLAGS) core/polyfill/strings.cpp -o core/polyfill/strings.o

core/polyfill/date.o: core/polyfill/date.cpp
	g++ -c $(CFLAGS) core/polyfill/date.cpp -o core/polyfill/date.o

core/polyfill/mimetype.o: core/polyfill/mimetype.cpp
	g++ -c $(CFLAGS) core/polyfill/mimetype.cpp -o core/polyfill/mimetype.o


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


# encoding stuff
$(LIB_CORE_ENCODING): $(LIB_CORE_ENCODING_DEPS)
	ar rvs $(LIB_CORE_ENCODING) $(LIB_CORE_ENCODING_DEPS)

core/encoding/base64.o: core/encoding/base64.cpp
	g++ -c $(CFLAGS) core/encoding/base64.cpp -o core/encoding/base64.o

core/encoding/hex.o: core/encoding/hex.cpp
	g++ -c $(CFLAGS) core/encoding/hex.cpp -o core/encoding/hex.o

core/encoding/url.o: core/encoding/url.cpp
	g++ -c $(CFLAGS) core/encoding/url.cpp -o core/encoding/url.o


# network stuff
$(LIB_CORE_NETWORK): $(LIB_CORE_NETWORK_DEPS)
	ar rvs $(LIB_CORE_NETWORK) $(LIB_CORE_NETWORK_DEPS)

core/network/tcp/connection.o: core/network/tcp/connection.cpp
	g++ -c $(CFLAGS) core/network/tcp/connection.cpp -o core/network/tcp/connection.o

core/network/tcp/listen.o: core/network/tcp/listener.cpp
	g++ -c $(CFLAGS) core/network/tcp/listener.cpp -o core/network/tcp/listen.o


# compression stuff
$(LIB_CORE_COMPRESS): $(LIB_CORE_COMPRESS_DEPS)
	ar rvs $(LIB_CORE_COMPRESS) $(LIB_CORE_COMPRESS_DEPS)

core/compression/streams.o: core/compression/streams.cpp
	g++ -c $(CFLAGS) core/compression/streams.cpp -o core/compression/streams.o

core/compression/brotli.o: core/compression/brotli.cpp
	g++ -c $(CFLAGS) core/compression/brotli.cpp -o core/compression/brotli.o

core/compression/zlib.o: core/compression/zlib.cpp
	g++ -c $(CFLAGS) core/compression/zlib.cpp -o core/compression/zlib.o


# server stuff
$(LIB_CORE_SERVER): $(LIB_CORE_SERVER_DEPS)
	ar rvs $(LIB_CORE_SERVER) $(LIB_CORE_SERVER_DEPS)

core/server/server.o: core/server/server.cpp
	g++ -c $(CFLAGS) core/server/server.cpp -o core/server/server.o

core/server/http/transport.o: core/server/http/transport.cpp
	g++ -c $(CFLAGS) core/server/http/transport.cpp -o core/server/http/transport.o

core/server/handlers/serverless.o: core/server/handlers/serverless.cpp
	g++ -c $(CFLAGS) core/server/handlers/serverless.cpp -o core/server/handlers/serverless.o

core/server/handlers/connection.o: core/server/handlers/connection.cpp
	g++ -c $(CFLAGS) core/server/handlers/connection.cpp -o core/server/handlers/connection.o

core/server/http/queue.o: core/server/http/queue.cpp
	g++ -c $(CFLAGS) core/server/http/queue.cpp -o core/server/http/queue.o

core/server/console.o: core/server/console.cpp
	g++ -c $(CFLAGS) core/server/console.cpp -o core/server/console.o


# html templates and stuff
$(LIB_CORE_HTML): $(LIB_CORE_HTML_DEPS)
	ar rvs $(LIB_CORE_HTML) $(LIB_CORE_HTML_DEPS)

core/html/templates.o: core/html/templates.cpp
	g++ -c $(CFLAGS) core/html/templates.cpp -o core/html/templates.o

core/html/engine.o: core/html/engine.cpp
	g++ -c $(CFLAGS) core/html/engine.cpp -o core/html/engine.o

core/html/resources/servicepage.res: core/html/resources/servicepage.html
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 core/html/resources/servicepage.html core/html/resources/servicepage.res


# jisson stuff
$(LIB_CORE_JSON): $(LIB_CORE_JSON_DEPS)
	ar rvs $(LIB_CORE_JSON) $(LIB_CORE_JSON_DEPS)

core/json/parse.o: core/json/parse.cpp
	g++ -c $(CFLAGS) core/json/parse.cpp -o core/json/parse.o

core/json/stringify.o: core/json/stringify.cpp
	g++ -c $(CFLAGS) core/json/stringify.cpp -o core/json/stringify.o

core/json/property.o: core/json/property.cpp
	g++ -c $(CFLAGS) core/json/property.cpp -o core/json/property.o


# crypto stuff
$(LIB_CORE_CRYPTO): $(LIB_CORE_CRYPTO_DEPS)
	ar rvs $(LIB_CORE_CRYPTO) $(LIB_CORE_CRYPTO_DEPS)

core/crypto/sha1.o: core/crypto/sha1.cpp
	g++ -c $(CFLAGS) core/crypto/sha1.cpp -o core/crypto/sha1.o

core/crypto/uid.o: core/crypto/uid.cpp
	g++ -c $(CFLAGS) core/crypto/uid.cpp -o core/crypto/uid.o


# error handling
$(LIB_CORE_ERROR): $(LIB_CORE_ERROR_DEPS)
	ar rvs $(LIB_CORE_ERROR) $(LIB_CORE_ERROR_DEPS)

core/error/apierror.o: core/error/apierror.cpp
	g++ -c $(CFLAGS) core/error/apierror.cpp -o core/error/apierror.o
