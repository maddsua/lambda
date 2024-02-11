
# Test zlib compression module
TEST_ZLIB_TARGET = $(EXEPFX)zlib.test$(EXEEXT)
test.zlib: $(TEST_ZLIB_TARGET)
	$(TEST_ZLIB_TARGET)

$(TEST_ZLIB_TARGET): tests/zlib/zlib.test.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) tests/zlib/zlib.test.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_ZLIB_TARGET)

tests/zlib/zlib.test.o: tests/zlib/zlib.test.cpp
	g++ -c $(CFLAGS) tests/zlib/zlib.test.cpp -o tests/zlib/zlib.test.o
