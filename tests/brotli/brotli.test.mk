
# Test brotli compression module
TEST_BROTLI_TARGET = $(EXEPFX)brotli.test$(EXEEXT)
test.brotli: $(TEST_BROTLI_TARGET)
	$(TEST_BROTLI_TARGET)

$(TEST_BROTLI_TARGET): tests/brotli/brotli.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) tests/brotli/brotli.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_BROTLI_TARGET)

tests/brotli/brotli.test.o: tests/brotli/brotli.test.cpp
	g++ -c $(CFLAGS) tests/brotli/brotli.test.cpp -o tests/brotli/brotli.test.o
