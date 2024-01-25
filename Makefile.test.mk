
# Test URL core module
test.url: url.test$(EXEEXT)

url.test$(EXEEXT): test/url.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/url.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o url.test$(EXEEXT)

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


# Test encoding core module
test.encoding: encoding.test$(EXEEXT)

encoding.test$(EXEEXT): test/encoding.test.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/encoding.test.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o encoding.test$(EXEEXT)

test/encoding.test.o: test/encoding.test.cpp
	g++ -c $(CFLAGS) test/encoding.test.cpp -o test/encoding.test.o


# Test JSON extra module
test.json: json.test$(EXEEXT)

json.test$(EXEEXT): test/json.test.o $(LIB_EXTRA_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/json.test.cpp $(LIB_EXTRA_JSON) $(LIB_CORE_POLYFILL) -o json.test$(EXEEXT)

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
test.storage: storage.test$(EXEEXT)

storage.test$(EXEEXT): test/storage.test.o $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/storage.test.cpp $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS) -o storage.test$(EXEEXT)

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test zlib compression module
test.zlib: zlib.test$(EXEEXT)

zlib.test$(EXEEXT): test/zlib.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/zlib.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(LINK_COMPRESS_LIBS) -o zlib.test$(EXEEXT)

test/zlib.test.o: test/zlib.test.cpp
	g++ -c $(CFLAGS) test/zlib.test.cpp -o test/zlib.test.o


# Test brotli compression module
test.brotli: brotli.test$(EXEEXT)

brotli.test$(EXEEXT): test/brotli.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/brotli.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(LINK_COMPRESS_LIBS) -o brotli.test$(EXEEXT)

test/brotli.test.o: test/brotli.test.cpp
	g++ -c $(CFLAGS) test/brotli.test.cpp -o test/brotli.test.o


# Test network/TCP core module
test.tcp: tcp.test$(EXEEXT)

tcp.test$(EXEEXT): test/tcp.test.o $(LIB_CORE_NETWORK)
	g++ $(CFLAGS) test/tcp.test.cpp $(LIB_CORE_NETWORK) -lws2_32 -o tcp.test$(EXEEXT)

test/tcp.test.o: test/tcp.test.cpp
	g++ -c $(CFLAGS) test/tcp.test.cpp -o test/tcp.test.o

# Test network/transport/http core module
test.httptransport: httptransport.test$(EXEEXT)

httptransport.test$(EXEEXT): test/httptransport.test.o $(LIB_CORE_DEPS)
	g++ $(CFLAGS) test/httptransport.test.cpp $(LIB_CORE_DEPS) $(LINK_COMPRESS_LIBS) -lws2_32 -o httptransport.test$(EXEEXT)

test/httptransport.test.o: test/httptransport.test.cpp
	g++ -c $(CFLAGS) test/httptransport.test.cpp -o test/httptransport.test.o


# Test http server core module
test.httpserver: httpserver.test$(EXEEXT)

httpserver.test$(EXEEXT): test/httpserver.test.o $(LIB_CORE_DEPS)
	g++ $(CFLAGS) test/httpserver.test.cpp $(LIB_CORE_DEPS) $(LINK_COMPRESS_LIBS) -lws2_32 -o httpserver.test$(EXEEXT)

test/httpserver.test.o: test/httpserver.test.cpp
	g++ -c $(CFLAGS) test/httpserver.test.cpp -o test/httpserver.test.o

# Test polyfill/strings core module
test.strings: strings.test$(EXEEXT)

strings.test$(EXEEXT): test/strings.test.o $(LIB_CORE_POLYFILL_DEPS)
	g++ $(CFLAGS) test/strings.test.cpp $(LIB_CORE_POLYFILL_DEPS) -o strings.test$(EXEEXT)

test/strings.test.o: test/strings.test.cpp
	g++ -c $(CFLAGS) test/strings.test.cpp -o test/strings.test.o
