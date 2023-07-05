
LIBNAME			= lambda
APP_DEV			= $(LIBNAME).exe
CFLAGS			= -std=c++20 -g
LIBS_SHARED		= -lz -lbrotlicommon -lbrotlidec -lbrotlienc

#	to get these static libs you need to compile brotli and zlib youself
#	it's basically just compiling all source files to objects
#	and then putting all of them into .a static lib
LIBS_STATIC	= -l:libz.a -l:libbrotli.a
LIBS_SYSTEM	= -lws2_32 -lwinmm


LIB_BR_SHARED		= -lbrotlicommon -lbrotlidec -lbrotlienc
LIB_ZLIB_SHARED		= -lz

#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(APP_DEV) all-after

clean: action-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res


#--------------------------------
#--------------------------------
#
#	Library components section
#
#--------------------------------
#--------------------------------


#------------
# Component: Encoding
#------------

COMPONENT_ENCODING = obj_encoding
LIBSTATIC_ENCODING = lib$(LIBNAME)-encoding.a
OBJECTS_ENCODING = encoding/urlencode.o encoding/base64.o encoding/hex.o

$(COMPONENT_ENCODING): $(OBJECTS_ENCODING)
	ar rvs $(LIBSTATIC_ENCODING) $(OBJECTS_ENCODING)

encoding/urlencode.o: encoding/urlencode.cpp
	g++ -c encoding/urlencode.cpp -o encoding/urlencode.o $(CFLAGS)

encoding/base64.o: encoding/base64.cpp
	g++ -c encoding/base64.cpp -o encoding/base64.o $(CFLAGS)

encoding/hex.o: encoding/hex.cpp
	g++ -c encoding/hex.cpp -o encoding/hex.o $(CFLAGS)


#------------
# Component: HTTP
#------------

COMPONENT_HTTP = obj_http
LIBSTATIC_HTTP = lib$(LIBNAME)-http.a
OBJECTS_HTTP = http/strings.o http/headers.o http/urlsearchparams.o http/statuscode.o http/response.o http/request.o http/url.o http/mimetype.o http/time.o http/cookies.o http/version.o

$(COMPONENT_HTTP): $(OBJECTS_HTTP)
	ar rvs $(LIBSTATIC_HTTP) $(OBJECTS_HTTP)
	
http/strings.o: http/strings.cpp
	g++ -c http/strings.cpp -o http/strings.o $(CFLAGS)

http/headers.o: http/headers.cpp
	g++ -c http/headers.cpp -o http/headers.o $(CFLAGS)

http/urlsearchparams.o: http/urlsearchparams.cpp
	g++ -c http/urlsearchparams.cpp -o http/urlsearchparams.o $(CFLAGS)

http/statuscode.o: http/statuscode.cpp
	g++ -c http/statuscode.cpp -o http/statuscode.o $(CFLAGS)

http/response.o: http/response.cpp
	g++ -c http/response.cpp -o http/response.o $(CFLAGS)

http/request.o: http/request.cpp
	g++ -c http/request.cpp -o http/request.o $(CFLAGS)

http/url.o: http/url.cpp
	g++ -c http/url.cpp -o http/url.o $(CFLAGS)

http/mimetype.o: http/mimetype.cpp
	g++ -c http/mimetype.cpp -o http/mimetype.o $(CFLAGS)

http/time.o: http/time.cpp
	g++ -c http/time.cpp -o http/time.o $(CFLAGS)

http/cookies.o: http/cookies.cpp
	g++ -c http/cookies.cpp -o http/cookies.o $(CFLAGS)

http/version.o: http/version.cpp
	g++ -c http/version.cpp -o http/version.o $(CFLAGS)


#------------
# Component: Compression
#------------

COMPONENT_COMPRESS		= obj_compress
LIBSTATIC_COMPRESS		= lib$(LIBNAME)-compress.a
OBJECTS_COMPRESS_BR		= compress/brotli.o
OBJECTS_COMPRESS_ZLIB	= compress/zlib.o
OBJECTS_COMPRESS		= $(OBJECTS_COMPRESS_ZLIB) $(OBJECTS_COMPRESS_BR)

$(COMPONENT_COMPRESS): $(OBJECTS_COMPRESS)
	ar rvs $(LIBSTATIC_COMPRESS) $(OBJECTS_COMPRESS)

compress/zlib.o: compress/zlib.cpp
	g++ -c compress/zlib.cpp -o compress/zlib.o $(CFLAGS)

compress/brotli.o: compress/brotli.cpp
	g++ -c compress/brotli.cpp -o compress/brotli.o $(CFLAGS)


#------------
# Component: Crypto
#------------

COMPONENT_CRYPTO = obj_crypto
LIBSTATIC_CRYPTO = lib$(LIBNAME)-crypto.a
OBJECTS_CRYPTO = crypto/random.o crypto/sha1.o

$(COMPONENT_CRYPTO): $(OBJECTS_CRYPTO)
	ar rvs $(LIBSTATIC_CRYPTO) $(OBJECTS_CRYPTO)

crypto/random.o: crypto/random.cpp
	g++ -c crypto/random.cpp -o crypto/random.o $(CFLAGS)

crypto/sha1.o: crypto/sha1.cpp
	g++ -c crypto/sha1.cpp -o crypto/sha1.o $(CFLAGS)


#------------
# Component: Network
#------------

COMPONENT_SOCKETS = obj_sockets
LIBSTATIC_SOCKETS = lib$(LIBNAME)-sockets.a
OBJECTS_SOCKETS = network/tcpListenSocket.o network/httpTransport.o network/httpServer.o network/websocket.o fetch/fetch.o

$(COMPONENT_SOCKETS): $(OBJECTS_SOCKETS)
	ar rvs $(LIBSTATIC_SOCKETS) $(OBJECTS_SOCKETS)

network/tcpListenSocket.o: network/tcpListenSocket.cpp
	g++ -c network/tcpListenSocket.cpp -o network/tcpListenSocket.o $(CFLAGS)

network/httpTransport.o: network/httpTransport.cpp
	g++ -c network/httpTransport.cpp -o network/httpTransport.o $(CFLAGS)

network/httpServer.o: network/httpServer.cpp
	g++ -c network/httpServer.cpp -o network/httpServer.o $(CFLAGS)

network/websocket.o: network/websocket.cpp
	g++ -c network/websocket.cpp -o network/websocket.o $(CFLAGS)

fetch/fetch.o: fetch/fetch.cpp
	g++ -c fetch/fetch.cpp -o fetch/fetch.o $(CFLAGS)


#------------
# Component: Server
#------------

COMPONENT_SERVER = obj_server
LIBSTATIC_SERVER = lib$(LIBNAME)-server.a
OBJECTS_SERVER = server/service.o server/logs.o server/handler.o

$(COMPONENT_SERVER): $(OBJECTS_SERVER)
	ar rvs $(LIBSTATIC_SERVER) $(OBJECTS_SERVER)

server/service.o: server/service.cpp
	g++ -c server/service.cpp -o server/service.o $(CFLAGS)

server/logs.o: server/logs.cpp
	g++ -c server/logs.cpp -o server/logs.o $(CFLAGS)

server/handler.o: server/handler.cpp
	g++ -c server/handler.cpp -o server/handler.o $(CFLAGS)


#------------
# Component: Storage
#------------

COMPONENT_STORAGE = obj_storage
LIBSTATIC_STORAGE = lib$(LIBNAME)-storage.a
OBJECTS_STORAGE = storage/kv.o

$(COMPONENT_STORAGE): $(OBJECTS_STORAGE)
	ar rvs $(LIBSTATIC_STORAGE) $(OBJECTS_STORAGE)

storage/kv.o: storage/kv.cpp
	g++ -c storage/kv.cpp -o storage/kv.o $(CFLAGS)


#------------
# Full library
#------------

LIBFULL_OBJS  = $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(OBJECTS_STORAGE)
LIBSTATIC     = lib$(LIBNAME).a
LIBSHARED     = $(LIBNAME).dll

libstatic: $(LIBSTATIC)

$(LIBSTATIC): $(LIBFULL_OBJS)
	ar rvs $(LIBSTATIC) $(LIBFULL_OBJS)

libshared: $(LIBSHARED)

$(LIBSHARED): $(LIBFULL_OBJS) $(LIBNAME).res
	g++ $(LIBFULL_OBJS) $(LIBNAME).res $(LIBS_SHARED) $(LIBS_SYSTEM) $(CFLAGS) -s -shared -o $(LIBSHARED) -Wl,--out-implib,lib$(LIBSHARED).a

$(LIBNAME).res: $(LIBNAME).rc
	windres -i $(LIBNAME).rc --input-format=rc -o $(LIBNAME).res -O coff 


#--------------------------------
#--------------------------------
#
#	Component tests section
#
#--------------------------------
#--------------------------------


#------------
# Test: Encoding
#------------

test_encode: $(OBJECTS_ENCODING)
	g++ tests/encoding.cpp $(OBJECTS_ENCODING) -o test_encode.exe


#------------
# Test: HTTP
#------------

test_http: $(OBJECTS_HTTP) $(OBJECTS_ENCODING)
	g++ tests/http.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) -o test_http.exe


#------------
# Test: Brotli
#------------

test_brotli: $(OBJECTS_COMPRESS_BR)
	g++ tests/brotli.cpp $(OBJECTS_COMPRESS_BR) $(LIB_BR_SHARED) -o test_brotli.exe


#------------
# Test: zlib
#------------

test_zlib: $(OBJECTS_COMPRESS_ZLIB)
	g++ tests/zlib.cpp $(OBJECTS_COMPRESS_ZLIB) $(LIB_ZLIB_SHARED) -o test_zlib.exe


#------------
# Test: Server
#------------

test_server: $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO)
	g++ tests/server.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(LIBS_SHARED) $(LIBS_SYSTEM) -o test_server.exe


#------------
# Test: Fetch
#------------

test_fetch: $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO)
	g++ tests/fetch.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(LIBS_SHARED) $(LIBS_SYSTEM) -o test_fetch.exe


#------------
# Test: KV Storage
#------------

test_kv: $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS)
	g++ tests/kv.cpp $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS) $(LIBS_SHARED) -o test_kv.exe

