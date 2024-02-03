
# Test URL core module
TEST_URL_TARGET = $(EXEPFX)url.test$(EXEEXT)
test.url: $(TEST_URL_TARGET)
	$(TEST_URL_TARGET)

$(TEST_URL_TARGET): test/url/main.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/url/main.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_URL_TARGET)

test/url/main.o: test/url/main.cpp
	g++ -c $(CFLAGS) test/url/main.cpp -o test/url/main.o
