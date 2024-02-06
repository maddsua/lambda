
# Test Storage extra module
TEST_STORAGE_TARGET = $(EXEPFX)storage.test$(EXEEXT)
TEST_STORAGE_DEPS	= $(LIB_EXTRA_KVSTORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_UTILS_DEPS)
test.storage: $(TEST_STORAGE_TARGET)
	$(TEST_STORAGE_TARGET)

$(TEST_STORAGE_TARGET): tests/storage/main.o $(TEST_STORAGE_DEPS)
	g++ $(CFLAGS) tests/storage/main.cpp $(TEST_STORAGE_DEPS) -o $(TEST_STORAGE_TARGET)

tests/storage/main.o: tests/storage/main.cpp
	g++ -c $(CFLAGS) tests/storage/main.cpp -o tests/storage/main.o
