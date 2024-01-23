
LIB_CORE				=	core/core.a
LIB_CORE_DEPS			=	$(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_HTTP_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_NETWORK_DEPS) $(LIB_CORE_COMPRESS_DEPS) $(LIB_CORE_SERVER_DEPS) $(LIB_CORE_CRYPTO_DEPS)

LIB_CORE_POLYFILL		=	core/polyfill.a
LIB_CORE_POLYFILL_DEPS	=	core/polyfill/strings.o core/polyfill/date.o core/polyfill/mimetype.o core/polyfill/uid.o

LIB_CORE_HTTP			=	core/http.a
LIB_CORE_HTTP_DEPS		=	core/http/cookies.o core/http/kvcontainer.o core/http/url.o core/http/urlsearchparams.o core/http/method.o core/http/status.o

LIB_CORE_ENCODING		=	core/encoding.a
LIB_CORE_ENCODING_DEPS	=	core/encoding/base64.o core/encoding/hex.o core/encoding/url.o

LIB_CORE_NETWORK		=	core/network.a
LIB_CORE_NETWORK_DEPS	=	core/network/tcpconn.o

LIB_CORE_COMPRESS		=	core/compression.a
LIB_CORE_COMPRESS_DEPS	=	core/compression/streams.o core/compression/brotli.o core/compression/zlib.o

LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_RESS	=	core/server/resources/html/servicepage.res
LIB_CORE_SERVER_OBJS	=	core/server/httpHandler.o core/server/httpServer.o core/server/serviceResponse.o core/server/console.o
LIB_CORE_SERVER_DEPS	=	$(LIB_CORE_SERVER_OBJS) $(LIB_CORE_SERVER_RESS)

LIB_CORE_CRYPTO			=	core/crypto.a
LIB_CORE_CRYPTO_DEPS	=	core/crypto/sha1.o


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

core/polyfill/uid.o: core/polyfill/uid.cpp
	g++ -c $(CFLAGS) core/polyfill/uid.cpp -o core/polyfill/uid.o


# http stuff
$(LIB_CORE_HTTP): $(LIB_CORE_HTTP_DEPS)
	ar rvs $(LIB_CORE_HTTP) $(LIB_CORE_HTTP_DEPS)

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

core/network/tcpconn.o: core/network/tcpconn.cpp
	g++ -c $(CFLAGS) core/network/tcpconn.cpp -o core/network/tcpconn.o


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

core/server/httpHandler.o: core/server/httpHandler.cpp
	g++ -c $(CFLAGS) core/server/httpHandler.cpp -o core/server/httpHandler.o

core/server/httpServer.o: core/server/httpServer.cpp
	g++ -c $(CFLAGS) core/server/httpServer.cpp -o core/server/httpServer.o

core/server/serviceResponse.o: core/server/serviceResponse.cpp
	g++ -c $(CFLAGS) core/server/serviceResponse.cpp -o core/server/serviceResponse.o

core/server/console.o: core/server/console.cpp
	g++ -c $(CFLAGS) core/server/console.cpp -o core/server/console.o

core/server/resources/html/servicepage.res: core/server/resources/html/servicepage.html
	objcopy --input-target binary --output-target elf64-x86-64 --binary-architecture i386 core/server/resources/html/servicepage.html core/server/resources/html/servicepage.res


# crypto stuff
$(LIB_CORE_CRYPTO): $(LIB_CORE_CRYPTO_DEPS)
	ar rvs $(LIB_CORE_CRYPTO) $(LIB_CORE_CRYPTO_DEPS)

core/crypto/sha1.o: core/crypto/sha1.cpp
	g++ -c $(CFLAGS) core/crypto/sha1.cpp -o core/crypto/sha1.o
