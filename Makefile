
LIBNAME  = lambda
APP_DEV    = $(LIBNAME).exe

FLAGS		= -std=c++20 -g
LIBS_SHARED	= -lz -lbrotlicommon -lbrotlidec -lbrotlienc

#	to get these static libs you need to compile brotli and zlib youself
#	it's basically just compiling all source files to objects
#	and then putting all of them into .a static lib
LIBS_STATIC	= -l:libz.a -l:libbrotli.a
LIBS_SYSTEM	= -lws2_32 -lwinmm

#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(APP_DEV) all-after

clean: action-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res


#------------
# Component: Encoding
#------------

COMPONENT_ENCODING = obj_encoding
LIBSTATIC_ENCODING = lib$(LIBNAME)-encoding.a
OBJECTS_ENCODING = encoding/urlencode.o encoding/base64.o encoding/hex.o

$(COMPONENT_ENCODING): $(OBJECTS_ENCODING)
	ar rvs $(LIBSTATIC_ENCODING) $(OBJECTS_ENCODING)

encoding/urlencode.o: encoding/urlencode.cpp
	g++ -c encoding/urlencode.cpp -o encoding/urlencode.o $(FLAGS)

encoding/base64.o: encoding/base64.cpp
	g++ -c encoding/base64.cpp -o encoding/base64.o $(FLAGS)

encoding/hex.o: encoding/hex.cpp
	g++ -c encoding/hex.cpp -o encoding/hex.o $(FLAGS)


#------------
# Test: Encoding
#------------

test_encode: $(OBJECTS_ENCODING)
	g++ tests/encoding.cpp $(OBJECTS_ENCODING) -o test_encode.exe


#------------
# Component: HTTP
#------------

COMPONENT_HTTP = obj_http
LIBSTATIC_HTTP = lib$(LIBNAME)-http.a
OBJECTS_HTTP = http/strings.o http/headers.o http/urlsearchparams.o http/statuscode.o http/response.o http/request.o http/url.o http/mimetype.o http/time.o http/cookies.o http/version.o

$(COMPONENT_HTTP): $(OBJECTS_HTTP)
	ar rvs $(LIBSTATIC_HTTP) $(OBJECTS_HTTP)
	
http/strings.o: http/strings.cpp
	g++ -c http/strings.cpp -o http/strings.o $(FLAGS)

http/headers.o: http/headers.cpp
	g++ -c http/headers.cpp -o http/headers.o $(FLAGS)

http/urlsearchparams.o: http/urlsearchparams.cpp
	g++ -c http/urlsearchparams.cpp -o http/urlsearchparams.o $(FLAGS)

http/statuscode.o: http/statuscode.cpp
	g++ -c http/statuscode.cpp -o http/statuscode.o $(FLAGS)

http/response.o: http/response.cpp
	g++ -c http/response.cpp -o http/response.o $(FLAGS)

http/request.o: http/request.cpp
	g++ -c http/request.cpp -o http/request.o $(FLAGS)

http/url.o: http/url.cpp
	g++ -c http/url.cpp -o http/url.o $(FLAGS)

http/mimetype.o: http/mimetype.cpp
	g++ -c http/mimetype.cpp -o http/mimetype.o $(FLAGS)

http/time.o: http/time.cpp
	g++ -c http/time.cpp -o http/time.o $(FLAGS)

http/cookies.o: http/cookies.cpp
	g++ -c http/cookies.cpp -o http/cookies.o $(FLAGS)

http/version.o: http/version.cpp
	g++ -c http/version.cpp -o http/version.o $(FLAGS)

#------------
# Test: HTTP
#------------

test_http: $(OBJECTS_HTTP) $(OBJECTS_ENCODING)
	g++ tests/http.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) -o test_http.exe


#------------
# Component: Compression
#------------

COMPONENT_COMPRESS = obj_compress
LIBSTATIC_COMPRESS = lib$(LIBNAME)-compress.a
OBJECTS_COMPRESS = compress/zlib.o compress/brotli.o

$(COMPONENT_COMPRESS): $(OBJECTS_COMPRESS)
	ar rvs $(LIBSTATIC_COMPRESS) $(OBJECTS_COMPRESS)

compress/zlib.o: compress/zlib.cpp
	g++ -c compress/zlib.cpp -o compress/zlib.o $(FLAGS)

compress/brotli.o: compress/brotli.cpp
	g++ -c compress/brotli.cpp -o compress/brotli.o $(FLAGS)


#------------
# Test: Compression
#------------

test_compress: $(OBJECTS_COMPRESS)
	g++ tests/compress.cpp $(OBJECTS_COMPRESS) $(LIBS_SHARED) -o test_compress.exe


#------------
# Component: Crypto
#------------

COMPONENT_CRYPTO = obj_crypto
LIBSTATIC_CRYPTO = lib$(LIBNAME)-crypto.a
OBJECTS_CRYPTO = crypto/random.o crypto/sha1.o crypto/sha256.o crypto/sha512.o

$(COMPONENT_CRYPTO): $(OBJECTS_CRYPTO)
	ar rvs $(LIBSTATIC_CRYPTO) $(OBJECTS_CRYPTO)

crypto/random.o: crypto/random.cpp
	g++ -c crypto/random.cpp -o crypto/random.o $(FLAGS)

crypto/sha1.o: crypto/sha1.cpp
	g++ -c crypto/sha1.cpp -o crypto/sha1.o $(FLAGS)

crypto/sha256.o: crypto/sha256.cpp
	g++ -c crypto/sha256.cpp -o crypto/sha256.o $(FLAGS)

crypto/sha512.o: crypto/sha512.cpp
	g++ -c crypto/sha512.cpp -o crypto/sha512.o $(FLAGS)



#------------
# Component: Network
#------------

COMPONENT_SOCKETS = obj_sockets
LIBSTATIC_SOCKETS = lib$(LIBNAME)-sockets.a
OBJECTS_SOCKETS = network/tcpListenSocket.o network/httpTransport.o network/httpServer.o network/websocket.o network/fetch.o

$(COMPONENT_SOCKETS): $(OBJECTS_SOCKETS)
	ar rvs $(LIBSTATIC_SOCKETS) $(OBJECTS_SOCKETS)

network/tcpListenSocket.o: network/tcpListenSocket.cpp
	g++ -c network/tcpListenSocket.cpp -o network/tcpListenSocket.o $(FLAGS)

network/httpTransport.o: network/httpTransport.cpp
	g++ -c network/httpTransport.cpp -o network/httpTransport.o $(FLAGS)

network/httpServer.o: network/httpServer.cpp
	g++ -c network/httpServer.cpp -o network/httpServer.o $(FLAGS)

network/websocket.o: network/websocket.cpp
	g++ -c network/websocket.cpp -o network/websocket.o $(FLAGS)

network/fetch.o: network/fetch.cpp
	g++ -c network/fetch.cpp -o network/fetch.o $(FLAGS)


#------------
# Component: Server
#------------

COMPONENT_SERVER = obj_server
LIBSTATIC_SERVER = lib$(LIBNAME)-server.a
OBJECTS_SERVER = server/service.o server/logs.o server/handler.o

$(COMPONENT_SERVER): $(OBJECTS_SERVER)
	ar rvs $(LIBSTATIC_SERVER) $(OBJECTS_SERVER)

server/service.o: server/service.cpp
	g++ -c server/service.cpp -o server/service.o $(FLAGS)

server/logs.o: server/logs.cpp
	g++ -c server/logs.cpp -o server/logs.o $(FLAGS)

server/handler.o: server/handler.cpp
	g++ -c server/handler.cpp -o server/handler.o $(FLAGS)


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
# Component: KV Storage
#------------

COMPONENT_KVSTORAGE = obj_kvstorage
LIBSTATIC_KVSTORAGE = lib$(LIBNAME)-kvstorage.a
OBJECTS_KVSTORAGE = storage/kvstorage.o

$(COMPONENT_KVSTORAGE): $(OBJECTS_KVSTORAGE)
	ar rvs $(LIBSTATIC_KVSTORAGE) $(OBJECTS_KVSTORAGE)

storage/kvstorage.o: storage/kvstorage.cpp
	g++ -c storage/kvstorage.cpp -o storage/kvstorage.o $(FLAGS)


#------------
# Test: KV Storage
#------------

test_kvstorage: $(OBJECTS_KVSTORAGE)
	g++ tests/kvstorage.cpp $(OBJECTS_KVSTORAGE) -o test_kvstorage.exe


#------------
# Full library
#------------

LIBFULL_OBJS  = $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(OBJECTS_KVSTORAGE)
LIBSTATIC     = lib$(LIBNAME).a
LIBSHARED     = $(LIBNAME).dll

libstatic: $(LIBSTATIC)

$(LIBSTATIC): $(LIBFULL_OBJS)
	ar rvs $(LIBSTATIC) $(LIBFULL_OBJS)

libshared: $(LIBSHARED)

$(LIBSHARED): $(LIBFULL_OBJS) $(LIBNAME).res
	g++ $(LIBFULL_OBJS) $(LIBNAME).res $(LIBS_SHARED) $(LIBS_SYSTEM) $(FLAGS) -s -shared -o $(LIBSHARED) -Wl,--out-implib,lib$(LIBSHARED).a

$(LIBNAME).res: $(LIBNAME).rc
	windres -i $(LIBNAME).rc --input-format=rc -o $(LIBNAME).res -O coff 
