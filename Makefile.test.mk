
# Test JSON core module
test.json: json.test.exe

json.test.exe: test/json.test.o core/core.o extra/extra.o
	g++ test/json.test.cpp core/core.o extra/extra.o -o json.test.exe $(CFLAGS)

test/json.test.o: test/json.test.cpp
	g++ -c test/json.test.cpp -o test/json.test.o $(CFLAGS)
