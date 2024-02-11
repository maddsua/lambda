
# Test error handling module
TEST_ERRORS_TARGET = $(EXEPFX)errors.test$(EXEEXT)
test.errors: $(TEST_ERRORS_TARGET)
	$(TEST_ERRORS_TARGET)

$(TEST_ERRORS_TARGET): tests/errors/errors.test.o $(LIB_CORE_UTILS_DEPS)
	g++ $(CFLAGS) tests/errors/errors.test.cpp $(LIB_CORE_UTILS_DEPS) -o $(TEST_ERRORS_TARGET)

tests/errors/errors.test.o: tests/errors/errors.test.cpp
	g++ -c $(CFLAGS) tests/errors/errors.test.cpp -o tests/errors/errors.test.o
