
# Test polyfill/strings core module
TEST_STRINGS_TARGET = $(EXEPFX)strings.test$(EXEEXT)
test.strings: $(TEST_STRINGS_TARGET)
	$(TEST_STRINGS_TARGET)

$(TEST_STRINGS_TARGET): tests/strings/main.o $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) tests/strings/main.cpp $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STRINGS_TARGET)

tests/strings/main.o: tests/strings/main.cpp
	g++ -c $(CFLAGS) tests/strings/main.cpp -o tests/strings/main.o
