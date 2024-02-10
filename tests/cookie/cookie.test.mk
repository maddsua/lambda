
# Test cookie core/http module
TEST_COOKIE_TARGET = $(EXEPFX)cookie.test$(EXEEXT)
test.cookie: $(TEST_COOKIE_TARGET)
	$(TEST_COOKIE_TARGET)

$(TEST_COOKIE_TARGET): tests/cookie/main.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/cookie/main.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_COOKIE_TARGET)

tests/cookie/main.o: tests/cookie/main.cpp
	g++ -c $(CFLAGS) tests/cookie/main.cpp -o tests/cookie/main.o
