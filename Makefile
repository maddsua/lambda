# 2023 maddsua's lambda
# Demo/Test app
# makefile for windows build
# GCC-12.2-ucrt64 is in use

FRAMEWORK  = lambda

APP_DEV		= lambda.exe
LIBNAME		= lambda

OBJECTS		= src/constants.o src/util.o src/httpcore.o src/lambda.o src/fetch.o src/compress.o src/filesystem.o src/sha.o src/localdb.o

FLAGS		= -std=c++20
LIBS_SHARED	= -lz -lbrotlicommon -lbrotlidec -lbrotlienc

#	to get these static libs you need to compile brotli and zlib youself
#	it's basically just compiling all source files to objects
#	and then putting all of them into .a static lib
LIBS_STATIC	= -l:libz.a -l:libbrotli.a
LIBS_SYSTEM	= -lws2_32 -lwinmm

LIBSTATIC	= lib$(LIBNAME).a
LIBSHARED	= $(LIBNAME).dll

#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(APP_DEV) all-after

clean: action-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res

#run: action-custom
#	obj_http obj_compress obj_crypto


#------------
# Component: HTTP
#------------

COMPONENT_HTTP = obj_http
LIBSTATIC_HTTP = lib$(FRAMEWORK)http.a
OBJECTS_HTTP = http/strings.o http/headers.o http/searchquery.o http/statuscode.o http/response.o http/request.o http/url.o http/mimetype.o http/time.o

$(COMPONENT_HTTP): $(OBJECTS_HTTP)
	ar rvs $(LIBSTATIC_HTTP) $(OBJECTS_HTTP)
	
http/strings.o: http/strings.cpp
	g++ -c http/strings.cpp -o http/strings.o $(FLAGS)

http/headers.o: http/headers.cpp
	g++ -c http/headers.cpp -o http/headers.o $(FLAGS)

http/searchquery.o: http/searchquery.cpp
	g++ -c http/searchquery.cpp -o http/searchquery.o $(FLAGS)

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


#------------
# Test: HTTP
#------------

test_http: $(OBJECTS_HTTP)
	g++ tests/http/http.cpp $(OBJECTS_HTTP) -o test_http.exe


#------------
# Component: Compression
#------------

COMPONENT_COMPRESS = obj_compress
LIBSTATIC_COMPRESS = lib$(FRAMEWORK)compression.a
OBJECTS_COMPRESS = compression/zlib.o compression/brotli.o

$(COMPONENT_COMPRESS): $(OBJECTS_COMPRESS)
	ar rvs $(LIBSTATIC_COMPRESS) $(OBJECTS_COMPRESS)

compression/zlib.o: compression/zlib.cpp
	g++ -c compression/zlib.cpp -o compression/zlib.o $(FLAGS)

compression/brotli.o: compression/brotli.cpp
	g++ -c compression/brotli.cpp -o compression/brotli.o $(FLAGS)


#------------
# Test: Compression
#------------

test_compress: $(OBJECTS_COMPRESS)
	g++ tests/compression/compression.cpp $(OBJECTS_COMPRESS) $(LIBS_SHARED) -o test_compress.exe


#------------
# Component: Crypto
#------------

COMPONENT_CRYPTO = obj_crypto
LIBSTATIC_CRYPTO = lib$(FRAMEWORK)crypto.a
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
# Component: Sockets
#------------

COMPONENT_SOCKETS = obj_sockets
LIBSTATIC_SOCKETS = lib$(FRAMEWORK)sockets.a
OBJECTS_SOCKETS = sockets/httpListenSocket.o sockets/httpClientSocket.o

$(COMPONENT_SOCKETS): $(OBJECTS_SOCKETS)
	ar rvs $(LIBSTATIC_SOCKETS) $(OBJECTS_SOCKETS)

sockets/httpListenSocket.o: sockets/httpListenSocket.cpp
	g++ -c sockets/httpListenSocket.cpp -o sockets/httpListenSocket.o $(FLAGS)

sockets/httpClientSocket.o: sockets/httpClientSocket.cpp
	g++ -c sockets/httpClientSocket.cpp -o sockets/httpClientSocket.o $(FLAGS)


#------------
# Component: Server
#------------

COMPONENT_SERVER = obj_server
LIBSTATIC_SERVER = lib$(FRAMEWORK)server.a
OBJECTS_SERVER = server/http.o server/logs.o

$(COMPONENT_SERVER): $(OBJECTS_SERVER)
	ar rvs $(LIBSTATIC_SERVER) $(OBJECTS_SERVER)

server/http.o: server/http.cpp
	g++ -c server/http.cpp -o server/http.o $(FLAGS)

server/logs.o: server/logs.cpp
	g++ -c server/logs.cpp -o server/logs.o $(FLAGS)

#------------
# Test: Server
#------------

test_server: $(OBJECTS_HTTP) $(OBJECTS_COMPRESS) $(OBJECTS_SOCKETS) $(OBJECTS_SERVER)
	g++ tests/server/server.cpp $(OBJECTS_SERVER) $(OBJECTS_SOCKETS) $(OBJECTS_HTTP) $(OBJECTS_COMPRESS) $(LIBS_SHARED) $(LIBS_SYSTEM) -o test_server.exe















































# ----
#	labmda demo/test app
# ----
#	regular dev app
#$(APP_DEV): main.o $(OBJECTS)
#	g++ main.o $(OBJECTS) $(LIBS_SHARED) $(LIBS_SYSTEM) $(FLAGS) -o $(APP_DEV)

#	fully static build, version for the demo
#static: $(LIBSTATIC) main.o
#	g++ -static main.o -L. -l$(LIBNAME) $(LIBS_STATIC) $(LIBS_SYSTEM) $(FLAGS) -o $(APP_DEV)

#	dynamically linked to all the dlls
#dynamic: libshared main.o
#	g++ main.o -L. -l$(LIBNAME) $(FLAGS) -o $(APP_DEV)


# ----
#	dev main
# ----
#main.o: main.cpp
#	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	lib
# ----
#	make static lib
#libstatic: $(LIBSTATIC)

#$(LIBSTATIC): $(OBJECTS)
#	ar rvs $(LIBSTATIC) $(OBJECTS)

#	make dll
#libshared: $(LIBSHARED)

#$(LIBSHARED): $(OBJECTS) $(LIBNAME).res
#	g++ $(OBJECTS) $(LIBNAME).res $(LIBS_SHARED) $(LIBS_SYSTEM) $(FLAGS) -s -shared -o $(LIBSHARED) -Wl,--out-implib,lib$(LIBSHARED).a


# ----
#	resources
# ----
#$(LIBNAME).res: $(LIBNAME).rc
#	windres -i $(LIBNAME).rc --input-format=rc -o $(LIBNAME).res -O coff 


