
# Test URL core module
TEST_URL_TARGET = $(EXEPFX)url.test$(EXEEXT)
test.url: $(TEST_URL_TARGET)
	$(TEST_URL_TARGET)

$(TEST_URL_TARGET): test/url.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/url.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_URL_TARGET)

test/url.test.o: test/url.test.cpp
	g++ -c $(CFLAGS) test/url.test.cpp -o test/url.test.o


# Test encoding core module
TEST_ENCODING_TARGET = $(EXEPFX)encoding.test$(EXEEXT)
test.encoding: $(TEST_ENCODING_TARGET)
	$(TEST_ENCODING_TARGET)

$(TEST_ENCODING_TARGET): test/encoding.test.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/encoding.test.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o $(TEST_ENCODING_TARGET)

test/encoding.test.o: test/encoding.test.cpp
	g++ -c $(CFLAGS) test/encoding.test.cpp -o test/encoding.test.o


# Test polyfill/strings core module
TEST_STRINGS_TARGET = $(EXEPFX)strings.test$(EXEEXT)
test.strings: $(TEST_STRINGS_TARGET)
	$(TEST_STRINGS_TARGET)

$(TEST_STRINGS_TARGET): test/strings.test.o $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/strings.test.cpp $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STRINGS_TARGET)

test/strings.test.o: test/strings.test.cpp
	g++ -c $(CFLAGS) test/strings.test.cpp -o test/strings.test.o


# Test JSON extra module
TEST_JSON_TARGET = $(EXEPFX)json.test$(EXEEXT)
test.json: $(TEST_JSON_TARGET)
	$(TEST_JSON_TARGET)

$(TEST_JSON_TARGET): test/json.test.o $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/json.test.cpp $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL) -o $(TEST_JSON_TARGET)

test/json.test.o: test/json.test.cpp
	g++ -c $(CFLAGS) test/json.test.cpp -o test/json.test.o


# Test Storage extra module
TEST_STORAGE_TARGET = $(EXEPFX)storage.test$(EXEEXT)
test.storage: $(TEST_STORAGE_TARGET)
	$(TEST_STORAGE_TARGET)

$(TEST_STORAGE_TARGET): test/storage.test.o $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/storage.test.cpp $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STORAGE_TARGET)

test/storage.test.o: test/storage.test.cpp
	g++ -c $(CFLAGS) test/storage.test.cpp -o test/storage.test.o


# Test zlib compression module
TEST_ZLIB_TARGET = $(EXEPFX)zlib.test$(EXEEXT)
test.zlib: $(TEST_ZLIB_TARGET)
	$(TEST_ZLIB_TARGET)

$(TEST_ZLIB_TARGET): test/zlib.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/zlib.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_ZLIB_TARGET)

test/zlib.test.o: test/zlib.test.cpp
	g++ -c $(CFLAGS) test/zlib.test.cpp -o test/zlib.test.o


# Test brotli compression module
TEST_BROTLI_TARGET = $(EXEPFX)brotli.test$(EXEEXT)
test.brotli: $(TEST_BROTLI_TARGET)
	$(TEST_BROTLI_TARGET)

$(TEST_BROTLI_TARGET): test/brotli.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/brotli.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_BROTLI_TARGET)

test/brotli.test.o: test/brotli.test.cpp
	g++ -c $(CFLAGS) test/brotli.test.cpp -o test/brotli.test.o


# Test error handling module
TEST_ERRORS_TARGET = $(EXEPFX)errors.test$(EXEEXT)
test.errors: $(TEST_ERRORS_TARGET)
	$(TEST_ERRORS_TARGET)

$(TEST_ERRORS_TARGET): test/errors.test.o $(LIB_CORE_ERROR_DEPS)
	g++ $(CFLAGS) test/errors.test.cpp $(LIB_CORE_ERROR_DEPS) -o $(TEST_ERRORS_TARGET)

test/errors.test.o: test/errors.test.cpp
	g++ -c $(CFLAGS) test/errors.test.cpp -o test/errors.test.o


# Test network/TCP core module
test.tcp: tcp.test$(EXEEXT)

tcp.test$(EXEEXT): test/tcp.test.o $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR)
	g++ $(CFLAGS) test/tcp.test.cpp $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR) $(LINK_SYSTEM_LIBS) -o tcp.test$(EXEEXT)

test/tcp.test.o: test/tcp.test.cpp
	g++ -c $(CFLAGS) test/tcp.test.cpp -o test/tcp.test.o
