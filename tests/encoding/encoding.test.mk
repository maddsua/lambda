
# Test encoding core module
TEST_ENCODING_TARGET = $(EXEPFX)encoding.test$(EXEEXT)
test.encoding: $(TEST_ENCODING_TARGET)
	$(TEST_ENCODING_TARGET)

$(TEST_ENCODING_TARGET): tests/encoding/main.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/encoding/main.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o $(TEST_ENCODING_TARGET)

tests/encoding/main.o: tests/encoding/main.cpp
	g++ -c $(CFLAGS) tests/encoding/main.cpp -o tests/encoding/main.o
