
LIBNAME			= lambda
CFLAGS			= -std=c++20 -g
MAINTARGET		= libshared

#	To get static libs you need to compile brotli and zlib youself.
#	Head to deps dir, "run npm run build", then "make",
#	NodeJS is required for this build step

LIB_BR_SHARED		= -lbrotlicommon -lbrotlidec -lbrotlienc
LIB_BR_STATIC		= -L./deps/ -l:libbrotli-static.a
LIB_ZLIB_SHARED		= -lz
LIB_ZLIB_STATIC		= -L./deps/ -l:libz-static.a

LIBS_SYSTEM			= -lws2_32


#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(MAINTARGET) all-after

#------------
#	Utils
#------------

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
OBJECTS_ENCODING = encoding/urlencode.o encoding/base64.o encoding/hex.o encoding/json.o

encoding/urlencode.o: encoding/urlencode.cpp
	g++ -c encoding/urlencode.cpp -o encoding/urlencode.o $(CFLAGS)

encoding/base64.o: encoding/base64.cpp
	g++ -c encoding/base64.cpp -o encoding/base64.o $(CFLAGS)

encoding/hex.o: encoding/hex.cpp
	g++ -c encoding/hex.cpp -o encoding/hex.o $(CFLAGS)

encoding/json.o: encoding/json.cpp
	g++ -c encoding/json.cpp -o encoding/json.o $(CFLAGS)

#------------
# Component: HTTP
#------------
OBJECTS_HTTP = http/strings.o http/headers.o http/urlsearchparams.o http/statuscode.o http/response.o http/request.o http/url.o http/mimetype.o http/time.o http/cookies.o http/version.o
	
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
OBJECTS_COMPRESS_BR		= compress/brotli.o
OBJECTS_COMPRESS_ZLIB	= compress/zlib.o
OBJECTS_COMPRESS		= $(OBJECTS_COMPRESS_ZLIB) $(OBJECTS_COMPRESS_BR)

compress/zlib.o: compress/zlib.cpp
	g++ -c compress/zlib.cpp -o compress/zlib.o $(CFLAGS)

compress/brotli.o: compress/brotli.cpp
	g++ -c compress/brotli.cpp -o compress/brotli.o $(CFLAGS)


#------------
# Component: Crypto
#------------
OBJECTS_CRYPTO = crypto/random.o crypto/sha1.o

crypto/random.o: crypto/random.cpp
	g++ -c crypto/random.cpp -o crypto/random.o $(CFLAGS)

crypto/sha1.o: crypto/sha1.cpp
	g++ -c crypto/sha1.cpp -o crypto/sha1.o $(CFLAGS)


#------------
# Component: Network
#------------
OBJECTS_NETWORK = network/listenSocket.o network/baseConnection.o network/httpConnection.o network/websocket.o client/fetch.o

network/listenSocket.o: network/listenSocket.cpp
	g++ -c network/listenSocket.cpp -o network/listenSocket.o $(CFLAGS)

network/baseConnection.o: network/baseConnection.cpp
	g++ -c network/baseConnection.cpp -o network/baseConnection.o $(CFLAGS)

network/httpConnection.o: network/httpConnection.cpp
	g++ -c network/httpConnection.cpp -o network/httpConnection.o $(CFLAGS)

network/websocket.o: network/websocket.cpp
	g++ -c network/websocket.cpp -o network/websocket.o $(CFLAGS)

client/fetch.o: client/fetch.cpp
	g++ -c client/fetch.cpp -o client/fetch.o $(CFLAGS)


#------------
# Component: Server
#------------
SRC_RES_SVCPAGE = resources/html/servicepage
OBJECTS_SERVER = server/service.o server/logs.o server/handler.o $(SRC_RES_SVCPAGE).res server/responses.o

server/service.o: server/service.cpp
	g++ -c server/service.cpp -o server/service.o $(CFLAGS)

server/logs.o: server/logs.cpp
	g++ -c server/logs.cpp -o server/logs.o $(CFLAGS)

server/handler.o: server/handler.cpp
	g++ -c server/handler.cpp -o server/handler.o $(CFLAGS)

server/responses.o: server/responses.cpp
	g++ -c server/responses.cpp -o server/responses.o $(CFLAGS)

$(SRC_RES_SVCPAGE).res: $(SRC_RES_SVCPAGE).html
	objcopy --input-target binary --output-target elf64-x86-64 --binary-architecture i386 $(SRC_RES_SVCPAGE).html $(SRC_RES_SVCPAGE).res


#------------
# Component: Storage
#------------
OBJECTS_STORAGE = storage/kv.o storage/kv_db.o storage/kv_json.o storage/vfs.o storage/vfs_tar.o storage/vfs_lvfs2.o

storage/kv.o: storage/kv.cpp
	g++ -c storage/kv.cpp -o storage/kv.o $(CFLAGS)

storage/kv_db.o: storage/kv_db.cpp
	g++ -c storage/kv_db.cpp -o storage/kv_db.o $(CFLAGS)

storage/kv_json.o: storage/kv_json.cpp
	g++ -c storage/kv_json.cpp -o storage/kv_json.o $(CFLAGS)

storage/vfs.o: storage/vfs.cpp
	g++ -c storage/vfs.cpp -o storage/vfs.o $(CFLAGS)

storage/vfs_tar.o: storage/vfs_tar.cpp
	g++ -c storage/vfs_tar.cpp -o storage/vfs_tar.o $(CFLAGS)
	
storage/vfs_lvfs2.o: storage/vfs_lvfs2.cpp
	g++ -c storage/vfs_lvfs2.cpp -o storage/vfs_lvfs2.o $(CFLAGS)


#--------------------------------
#--------------------------------
#
#	Full library build
#
#--------------------------------
#--------------------------------

LIB_DEPS		= $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED)
LIB_DLLINFO		= resources/lib/$(LIBNAME)
LIBFULL_OBJS	= $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(OBJECTS_STORAGE)
LIBSTATIC		= lib$(LIBNAME).a
LIBSHARED		= $(LIBNAME).dll

libshared: $(LIBSHARED)

$(LIBSHARED): $(LIBFULL_OBJS) $(LIB_DLLINFO).res
	g++ $(LIBFULL_OBJS) $(LIB_DLLINFO).res $(LIB_DEPS) $(LIBS_SYSTEM) $(CFLAGS) -s -shared -o $(LIBSHARED) -Wl,--out-implib,lib$(LIBSHARED).a

$(LIB_DLLINFO).res: $(LIB_DLLINFO).rc
	windres -i $(LIB_DLLINFO).rc --input-format=rc -o $(LIB_DLLINFO).res -O coff

$(LIB_DLLINFO).rc: resources/lib/template.rc lambda_version.hpp
	node autogen_dll_info.mjs


libstatic: $(LIBSTATIC)

$(LIBSTATIC): $(LIBFULL_OBJS)
	ar rvs $(LIBSTATIC) $(LIBFULL_OBJS)


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
test_encoding: $(OBJECTS_ENCODING)
	g++ tests/encoding.cpp $(OBJECTS_ENCODING) -o test_encoding


#------------
# Test: HTTP
#------------
test_http: $(OBJECTS_HTTP) $(OBJECTS_ENCODING)
	g++ tests/http.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) -o test_http


#------------
# Test: Brotli
#------------
test_brotli: $(OBJECTS_COMPRESS_BR)
	g++ tests/brotli.cpp $(OBJECTS_COMPRESS_BR) $(LIB_BR_SHARED) -o test_brotli


#------------
# Test: zlib
#------------
test_zlib: $(OBJECTS_COMPRESS_ZLIB)
	g++ tests/zlib.cpp $(OBJECTS_COMPRESS_ZLIB) $(LIB_ZLIB_SHARED) -o test_zlib


#------------
# Test: Server
#------------
test_server: $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO)
	g++ tests/server.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED) $(LIBS_SYSTEM) -o test_server


#------------
# Test: Fetch
#------------
test_fetch: $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO)
	g++ tests/fetch.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED) $(LIBS_SYSTEM) -o test_fetch


#------------
# Test: Websocket client
#------------
test_websocket_client: $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO)
	g++ tests/websocket_client.cpp $(OBJECTS_HTTP) $(OBJECTS_ENCODING) $(OBJECTS_COMPRESS) $(OBJECTS_NETWORK) $(OBJECTS_SERVER) $(OBJECTS_CRYPTO) $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED) $(LIBS_SYSTEM) -o test_websocket_client


#------------
# Test: KV Storage
#------------
test_kv: $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS) $(OBJECTS_ENCODING)
	g++ tests/kv.cpp $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS) $(OBJECTS_ENCODING) $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED) -o test_kv

test_vfs: $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS) $(OBJECTS_ENCODING)
	g++ tests/vfs.cpp $(OBJECTS_STORAGE) $(OBJECTS_COMPRESS) $(OBJECTS_ENCODING) $(LIB_BR_SHARED) $(LIB_ZLIB_SHARED) -o test_vfs


#------------
# Test: JSON
#------------
test_json: encoding/json.o
	g++ tests/json.cpp encoding/json.o -o test_json


#--------------------------------
#--------------------------------
#
#	Example programs
#
#--------------------------------
#--------------------------------

#------------
# Simple api server demo
#------------
example_api_server: $(LIBSHARED)
	g++ examples/api_server.cpp $(LIBSHARED) $(LIBS_SYSTEM) $(CFLAGS) -o example_api_server

#------------
# Simple web server demo
#------------
example_web_server: $(LIBSHARED)
	g++ examples/web_server.cpp $(LIBSHARED) $(LIBS_SYSTEM) $(CFLAGS) -o example_web_server

#------------
# Demo app
#------------
example_demo: $(LIBSHARED)
	g++ examples/demo.cpp $(LIBSHARED) $(LIBS_SYSTEM) $(CFLAGS) -o example_demo
