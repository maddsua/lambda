
# Test URL core module
TEST_URL_TARGET = $(TEMPBIN)url.test$(EXEEXT)
test.url: $(TEST_URL_TARGET)
	$(TEST_URL_TARGET)

$(TEST_URL_TARGET): test/url.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/url.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_URL_TARGET)

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


# Test encoding core module
TEST_ENCODING_TARGET = $(TEMPBIN)encoding.test$(EXEEXT)
test.encoding: $(TEST_ENCODING_TARGET)
	$(TEST_ENCODING_TARGET)

$(TEST_ENCODING_TARGET): test/encoding.test.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/encoding.test.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o $(TEST_ENCODING_TARGET)

test/encoding.test.o: test/encoding.test.cpp
	g++ -c $(CFLAGS) test/encoding.test.cpp -o test/encoding.test.o


# Test polyfill/strings core module
TEST_STRINGS_TARGET = $(TEMPBIN)strings.test$(EXEEXT)
test.strings: $(TEST_STRINGS_TARGET)
	$(TEST_STRINGS_TARGET)

$(TEST_STRINGS_TARGET): test/strings.test.o $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/strings.test.cpp $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STRINGS_TARGET)

test/strings.test.o: test/strings.test.cpp
	g++ -c $(CFLAGS) test/strings.test.cpp -o test/strings.test.o


# Test JSON extra module
TEST_JSON_TARGET = $(TEMPBIN)json.test$(EXEEXT)
test.json: $(TEST_JSON_TARGET)
	$(TEST_JSON_TARGET)

$(TEST_JSON_TARGET): test/json.test.o $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/json.test.cpp $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL) -o $(TEST_JSON_TARGET)

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
TEST_STORAGE_TARGET = $(TEMPBIN)storage.test$(EXEEXT)
test.storage: $(TEST_STORAGE_TARGET)
	$(TEST_STORAGE_TARGET)

$(TEST_STORAGE_TARGET): test/storage.test.o $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/storage.test.cpp $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STORAGE_TARGET)

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test zlib compression module
TEST_ZLIB_TARGET = $(TEMPBIN)zlib.test$(EXEEXT)
test.zlib: $(TEST_ZLIB_TARGET)
	$(TEST_ZLIB_TARGET)

$(TEST_ZLIB_TARGET): test/zlib.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/zlib.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_ZLIB_TARGET)

test/zlib.test.o: test/zlib.test.cpp
	g++ -c $(CFLAGS) test/zlib.test.cpp -o test/zlib.test.o


# Test brotli compression module
TEST_BROTLI_TARGET = $(TEMPBIN)brotli.test$(EXEEXT)
test.brotli: $(TEST_BROTLI_TARGET)
	$(TEST_BROTLI_TARGET)

$(TEST_BROTLI_TARGET): test/brotli.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/brotli.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_BROTLI_TARGET)

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
	g++ $(CFLAGS) test/httptransport.test.cpp $(LIB_CORE_DEPS) $(EXTERNAL_LIBS) -lws2_32 -o httptransport.test$(EXEEXT)

test/httptransport.test.o: test/httptransport.test.cpp
	g++ -c $(CFLAGS) test/httptransport.test.cpp -o test/httptransport.test.o


# Test http server core module
test.httpserver: httpserver.test$(EXEEXT)

httpserver.test$(EXEEXT): test/httpserver.test.o $(LIB_CORE_DEPS)
	g++ $(CFLAGS) test/httpserver.test.cpp $(LIB_CORE_DEPS) $(EXTERNAL_LIBS) -lws2_32 -o httpserver.test$(EXEEXT)

test/httpserver.test.o: test/httpserver.test.cpp
	g++ -c $(CFLAGS) test/httpserver.test.cpp -o test/httpserver.test.o
