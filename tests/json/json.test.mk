
# Test JSON extra module
TEST_JSON_TARGET = $(EXEPFX)json.test$(EXEEXT)

test.json: $(TEST_JSON_TARGET)
	$(TEST_JSON_TARGET)

$(TEST_JSON_TARGET): tests/json/json.test.o $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) tests/json/json.test.cpp $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL) -o $(TEST_JSON_TARGET)

tests/json/json.test.o: tests/json/json.test.cpp
	g++ -c $(CFLAGS) tests/json/json.test.cpp -o tests/json/json.test.o
