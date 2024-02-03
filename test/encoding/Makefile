
# Test encoding core module
TEST_ENCODING_TARGET = $(EXEPFX)encoding.test$(EXEEXT)
test.encoding: $(TEST_ENCODING_TARGET)
	$(TEST_ENCODING_TARGET)

$(TEST_ENCODING_TARGET): test/encoding/main.o $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/encoding/main.cpp $(LIB_CORE_ENCODING) $(LIB_CORE_POLYFILL) -o $(TEST_ENCODING_TARGET)

test/encoding/main.o: test/encoding/main.cpp
	g++ -c $(CFLAGS) test/encoding/main.cpp -o test/encoding/main.o
