
# Test JSON extra module
TEST_JSON_TARGET = $(EXEPFX)json.test$(EXEEXT)
test.json: $(TEST_JSON_TARGET)
	$(TEST_JSON_TARGET)

$(TEST_JSON_TARGET): test/json/main.o $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL)
	g++ $(CFLAGS) test/json/main.cpp $(LIB_CORE_JSON) $(LIB_CORE_POLYFILL) -o $(TEST_JSON_TARGET)

test/json/main.o: test/json/main.cpp
	g++ -c $(CFLAGS) test/json/main.cpp -o test/json/main.o
