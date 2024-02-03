
# Test brotli compression module
TEST_BROTLI_TARGET = $(EXEPFX)brotli.test$(EXEEXT)
test.brotli: $(TEST_BROTLI_TARGET)
	$(TEST_BROTLI_TARGET)

$(TEST_BROTLI_TARGET): test/brotli/main.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/brotli/main.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_BROTLI_TARGET)

test/brotli/main.o: test/brotli/main.cpp
	g++ -c $(CFLAGS) test/brotli/main.cpp -o test/brotli/main.o
