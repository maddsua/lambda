
# Test encoding core module
TEST_ENCODING_TARGET = $(EXEPFX)encoding.test$(EXEEXT)
test.encoding: $(TEST_ENCODING_TARGET)
	$(TEST_ENCODING_TARGET)

$(TEST_ENCODING_TARGET): tests/encoding/encoding.test.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/encoding/encoding.test.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o $(TEST_ENCODING_TARGET)

tests/encoding/encoding.test.o: tests/encoding/encoding.test.cpp
	g++ -c $(CFLAGS) tests/encoding/encoding.test.cpp -o tests/encoding/encoding.test.o
