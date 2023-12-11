
# Test URL core module
test.url: url.test.exe

url.test.exe: test/url.test.o $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ)
	g++ $(CFLAGS) test/url.test.cpp $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ) -o url.test.exe

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


# Test encoding core module
test.encoding: encoding.test.exe

encoding.test.exe: test/encoding.test.o $(CORE_ENCODING_OBJ) $(CORE_POLYFILL_OBJ)
	g++ $(CFLAGS) test/encoding.test.cpp $(CORE_ENCODING_OBJ) $(CORE_POLYFILL_OBJ) -o encoding.test.exe

test/encoding.test.o: test/encoding.test.cpp
	g++ -c $(CFLAGS) test/encoding.test.cpp -o test/encoding.test.o


# Test JSON extra module
test.json: json.test.exe

json.test.exe: test/json.test.o $(CORE_JSON_OBJ) $(CORE_POLYFILL_OBJ)
	g++ $(CFLAGS) test/json.test.cpp $(CORE_JSON_OBJ) $(CORE_POLYFILL_OBJ) -o json.test.exe

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
test.storage: storage.test.exe

storage.test.exe: test/storage.test.o $(CORE_COMPRESS_OBJ) $(CORE_ENCODING_OBJ)
	g++ $(CFLAGS) test/storage.test.cpp $(CORE_COMPRESS_OBJ) $(CORE_ENCODING_OBJ) -o storage.test.exe

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test zlib compression module
test.zlib: zlib.test.exe

zlib.test.exe: test/zlib.test.o $(CORE_COMPRESS_OBJ)
	g++ $(CFLAGS) test/zlib.test.cpp $(CORE_COMPRESS_OBJ) $(CORE_COMPRESS_LIBS) -o zlib.test.exe

test/zlib.test.o: test/zlib.test.cpp
	g++ -c $(CFLAGS) test/zlib.test.cpp -o test/zlib.test.o


# Test brotli compression module
test.brotli: brotli.test.exe

brotli.test.exe: test/brotli.test.o $(CORE_COMPRESS_OBJ)
	g++ $(CFLAGS) test/brotli.test.cpp $(CORE_COMPRESS_OBJ) $(CORE_COMPRESS_LIBS) -o brotli.test.exe

test/brotli.test.o: test/brotli.test.cpp
	g++ -c $(CFLAGS) test/brotli.test.cpp -o test/brotli.test.o


# Test network/TCP core module
test.tcp: tcp.test.exe

tcp.test.exe: test/tcp.test.o $(CORE_NETWORK_OBJ)
	g++ $(CFLAGS) test/tcp.test.cpp $(CORE_NETWORK_OBJ) -lws2_32 -o tcp.test.exe

test/tcp.test.o: test/tcp.test.cpp
	g++ -c $(CFLAGS) test/tcp.test.cpp -o test/tcp.test.o

# Test network/transport/http core module
test.httpserver: httpserver.test.exe

httpserver.test.exe: test/httpserver.test.o $(CORE_SERVER_OBJ) $(CORE_NETWORK_OBJ) $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ)
	g++ $(CFLAGS) test/httpserver.test.cpp $(CORE_SERVER_OBJ) $(CORE_NETWORK_OBJ) $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ) -lws2_32 -o httpserver.test.exe

test/httpserver.test.o: test/httpserver.test.cpp
	g++ -c $(CFLAGS) test/httpserver.test.cpp -o test/httpserver.test.o
