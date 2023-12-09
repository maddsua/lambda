
# Test JSON extra module
test.json: json.test.exe

json.test.exe: test/json.test.o $(CORE_POLYFILL_OBJ) $(CORE_JSON_OBJ)
	g++ $(CFLAGS) test/json.test.cpp $(CORE_POLYFILL_OBJ) $(CORE_JSON_OBJ) -o json.test.exe

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
test.storage: storage.test.exe

storage.test.exe: test/storage.test.o $(CORE_COMPRESS_OBJ) $(CORE_ENCODING_OBJ)
	g++ $(CFLAGS) test/storage.test.cpp $(CORE_COMPRESS_OBJ) $(CORE_ENCODING_OBJ) -o storage.test.exe

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test URL core module
test.url: url.test.exe

url.test.exe: test/url.test.o $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ)
	g++ $(CFLAGS) test/url.test.cpp $(CORE_HTTP_OBJ) $(CORE_POLYFILL_OBJ) -o url.test.exe

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


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
