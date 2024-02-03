
# Test polyfill/strings core module
TEST_STRINGS_TARGET = $(EXEPFX)strings.test$(EXEEXT)
test.strings: $(TEST_STRINGS_TARGET)
	$(TEST_STRINGS_TARGET)

$(TEST_STRINGS_TARGET): test/strings/main.o $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS)
	g++ $(CFLAGS) test/strings/main.cpp $(LIB_CORE_POLYFILL_DEPS) $(LIB_CORE_ENCODING_DEPS) -o $(TEST_STRINGS_TARGET)

test/strings/main.o: test/strings/main.cpp
	g++ -c $(CFLAGS) test/strings/main.cpp -o test/strings/main.o
