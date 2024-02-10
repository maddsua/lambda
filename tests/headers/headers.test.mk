
# Test headers core/http module
TEST_HEADERS_TARGET = $(EXEPFX)headers.test$(EXEEXT)
test.headers: $(TEST_HEADERS_TARGET)
	$(TEST_HEADERS_TARGET)

$(TEST_HEADERS_TARGET): tests/headers/main.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/headers/main.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_HEADERS_TARGET)

tests/headers/main.o: tests/headers/main.cpp
	g++ -c $(CFLAGS) tests/headers/main.cpp -o tests/headers/main.o
