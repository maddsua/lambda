
# Test URL core module
test.url: url.test.exe

url.test.exe: test/url.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/url.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o url.test.exe

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


# Test encoding core module
test.encoding: encoding.test.exe

encoding.test.exe: test/encoding.test.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/encoding.test.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o encoding.test.exe

test/encoding.test.o: test/encoding.test.cpp
	g++ -c $(CFLAGS) test/encoding.test.cpp -o test/encoding.test.o


# Test JSON extra module
test.json: json.test.exe

json.test.exe: test/json.test.o $(LIB_EXTRA_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/json.test.cpp $(LIB_EXTRA_JSON) $(LIB_CORE_POLYFILL) -o json.test.exe

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
test.storage: storage.test.exe

storage.test.exe: test/storage.test.o $(LIB_EXTRA_STORAGE) $(LIB_CORE_ENCODING)
	g++ $(CFLAGS) test/storage.test.cpp $(LIB_EXTRA_STORAGE) $(LIB_CORE_ENCODING) -o storage.test.exe

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test zlib compression module
test.zlib: zlib.test.exe

zlib.test.exe: test/zlib.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/zlib.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(LINK_COMPRESS_LIBS) -o zlib.test.exe

test/zlib.test.o: test/zlib.test.cpp
	g++ -c $(CFLAGS) test/zlib.test.cpp -o test/zlib.test.o


# Test brotli compression module
test.brotli: brotli.test.exe

brotli.test.exe: test/brotli.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/brotli.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(LINK_COMPRESS_LIBS) -o brotli.test.exe

test/brotli.test.o: test/brotli.test.cpp
	g++ -c $(CFLAGS) test/brotli.test.cpp -o test/brotli.test.o


# Test network/TCP core module
test.tcp: tcp.test.exe

tcp.test.exe: test/tcp.test.o $(LIB_CORE_NETWORK)
	g++ $(CFLAGS) test/tcp.test.cpp $(LIB_CORE_NETWORK) -lws2_32 -o tcp.test.exe

test/tcp.test.o: test/tcp.test.cpp
	g++ -c $(CFLAGS) test/tcp.test.cpp -o test/tcp.test.o

# Test network/transport/http core module
test.httpserver: httpserver.test.exe

httpserver.test.exe: test/httpserver.test.o $(LIB_CORE_SERVER) $(LIB_CORE_NETWORK) $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/httpserver.test.cpp $(LIB_CORE_SERVER) $(LIB_CORE_NETWORK) $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) $(LIB_CORE_COMPRESS_DEPS) $(LINK_COMPRESS_LIBS) -lws2_32 -o httpserver.test.exe

test/httpserver.test.o: test/httpserver.test.cpp
	g++ -c $(CFLAGS) test/httpserver.test.cpp -o test/httpserver.test.o
