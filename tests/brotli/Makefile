
# Test brotli compression module
TEST_BROTLI_TARGET = $(EXEPFX)brotli.test$(EXEEXT)
test.brotli: $(TEST_BROTLI_TARGET)
	$(TEST_BROTLI_TARGET)

$(TEST_BROTLI_TARGET): tests/brotli/main.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) tests/brotli/main.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_BROTLI_TARGET)

tests/brotli/main.o: tests/brotli/main.cpp
	g++ -c $(CFLAGS) tests/brotli/main.cpp -o tests/brotli/main.o
