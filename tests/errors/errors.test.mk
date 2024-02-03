
# Test error handling module
TEST_ERRORS_TARGET = $(EXEPFX)errors.test$(EXEEXT)
test.errors: $(TEST_ERRORS_TARGET)
	$(TEST_ERRORS_TARGET)

$(TEST_ERRORS_TARGET): tests/errors/main.o $(LIB_CORE_UTILS_DEPS)
	g++ $(CFLAGS) tests/errors/main.cpp $(LIB_CORE_UTILS_DEPS) -o $(TEST_ERRORS_TARGET)

tests/errors/main.o: tests/errors/main.cpp
	g++ -c $(CFLAGS) tests/errors/main.cpp -o tests/errors/main.o
