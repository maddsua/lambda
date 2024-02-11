
# Test polyfill/strings core module
TEST_STRINGS_TARGET = $(EXEPFX)strings.test$(EXEEXT)
test.strings: $(TEST_STRINGS_TARGET)
	$(TEST_STRINGS_TARGET)

$(TEST_STRINGS_TARGET): tests/strings/strings.test.o $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) tests/strings/strings.test.cpp $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STRINGS_TARGET)

tests/strings/strings.test.o: tests/strings/strings.test.cpp
	g++ -c $(CFLAGS) tests/strings/strings.test.cpp -o tests/strings/strings.test.o
