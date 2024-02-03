
# Test zlib compression module
TEST_ZLIB_TARGET = $(EXEPFX)zlib.test$(EXEEXT)
test.zlib: $(TEST_ZLIB_TARGET)
	$(TEST_ZLIB_TARGET)

$(TEST_ZLIB_TARGET): tests/zlib/main.o $(LIB_CORE_COMPRESS_DEPS)
	g++ $(CFLAGS) tests/zlib/main.cpp $(LIB_CORE_COMPRESS_DEPS) $(EXTERNAL_LIBS) -o $(TEST_ZLIB_TARGET)

tests/zlib/main.o: tests/zlib/main.cpp
	g++ -c $(CFLAGS) tests/zlib/main.cpp -o tests/zlib/main.o
