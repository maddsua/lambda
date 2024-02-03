
# Test Storage extra module
TEST_STORAGE_TARGET = $(EXEPFX)storage.test$(EXEEXT)
TEST_STORAGE_DEPS	= $(LIB_EXTRA_STORAGE_DEPS) $(LIB_CORE_ENCODING_DEPS) $(LIB_CORE_UTILS_DEPS)
test.storage: $(TEST_STORAGE_TARGET)
	$(TEST_STORAGE_TARGET)

$(TEST_STORAGE_TARGET): test/storage/main.o $(TEST_STORAGE_DEPS)
	g++ $(CFLAGS) test/storage/main.cpp $(TEST_STORAGE_DEPS) -o $(TEST_STORAGE_TARGET)

test/storage/main.o: test/storage/main.cpp
	g++ -c $(CFLAGS) test/storage/main.cpp -o test/storage/main.o
