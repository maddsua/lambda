
# Test JSON extra module
TEST_JSON_TARGET = $(EXEPFX)json.test$(EXEEXT)

test.json: $(TEST_JSON_TARGET)
	$(TEST_JSON_TARGET)

$(TEST_JSON_TARGET): tests/json/main.o $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/json/main.cpp $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL) -o $(TEST_JSON_TARGET)

tests/json/main.o: tests/json/main.cpp
	g++ -c $(CFLAGS) tests/json/main.cpp -o tests/json/main.o
