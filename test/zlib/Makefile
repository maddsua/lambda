
# Test zlib compression module
TEST_ZLIB_TARGET = $(EXEPFX)zlib.test$(EXEEXT)
test.zlib: $(TEST_ZLIB_TARGET)
	$(TEST_ZLIB_TARGET)

$(TEST_ZLIB_TARGET): test/zlib/main.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) test/zlib/main.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_ZLIB_TARGET)

test/zlib/main.o: test/zlib/main.cpp
	g++ -c $(CFLAGS) test/zlib/main.cpp -o test/zlib/main.o
