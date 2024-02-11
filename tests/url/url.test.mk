
# Test URL core module
TEST_URL_TARGET = $(EXEPFX)url.test$(EXEEXT)
test.url: $(TEST_URL_TARGET)
	$(TEST_URL_TARGET)

$(TEST_URL_TARGET): tests/url/url.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/url/url.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_URL_TARGET)

tests/url/url.test.o: tests/url/url.test.cpp
	g++ -c $(CFLAGS) tests/url/url.test.cpp -o tests/url/url.test.o
