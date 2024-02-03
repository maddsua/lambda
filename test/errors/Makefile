
# Test error handling module
TEST_ERRORS_TARGET = $(EXEPFX)errors.test$(EXEEXT)
test.errors: $(TEST_ERRORS_TARGET)
	$(TEST_ERRORS_TARGET)

$(TEST_ERRORS_TARGET): test/errors/main.o $(LIB_CORE_ERROR_DEPS)
	g++ $(CFLAGS) test/errors/main.cpp $(LIB_CORE_ERROR_DEPS) -o $(TEST_ERRORS_TARGET)

test/errors/main.o: test/errors/main.cpp
	g++ -c $(CFLAGS) test/errors/main.cpp -o test/errors/main.o
