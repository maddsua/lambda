
# Test URL core module
TEST_URL_TARGET = $(EXEPFX)url.test$(EXEEXT)
test.url: $(TEST_URL_TARGET)
	$(TEST_URL_TARGET)

$(TEST_URL_TARGET): tests/url/main.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/url/main.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_URL_TARGET)

tests/url/main.o: tests/url/main.cpp
	g++ -c $(CFLAGS) tests/url/main.cpp -o tests/url/main.o
