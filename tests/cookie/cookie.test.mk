
# Test cookie core/http module
TEST_COOKIE_TARGET = $(EXEPFX)cookie.test$(EXEEXT)
test.cookie: $(TEST_COOKIE_TARGET)
	$(TEST_COOKIE_TARGET)

$(TEST_COOKIE_TARGET): tests/cookie/cookie.test.o $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/cookie/cookie.test.cpp $(LIB_CORE_HTTP) $(LIB_CORE_POLYFILL) -o $(TEST_COOKIE_TARGET)

tests/cookie/cookie.test.o: tests/cookie/cookie.test.cpp
	g++ -c $(CFLAGS) tests/cookie/cookie.test.cpp -o tests/cookie/cookie.test.o
