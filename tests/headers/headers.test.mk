
# Test headers core/http module
TEST_HEADERS_TARGET = $(EXEPFX)headers.test$(EXEEXT)
test.headers: $(TEST_HEADERS_TARGET)
	$(TEST_HEADERS_TARGET)

$(TEST_HEADERS_TARGET): tests/headers/headers.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/headers/headers.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_HEADERS_TARGET)

tests/headers/headers.test.o: tests/headers/headers.test.cpp
	g++ -c $(CFLAGS) tests/headers/headers.test.cpp -o tests/headers/headers.test.o
