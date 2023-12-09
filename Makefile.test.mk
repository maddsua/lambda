
# Test JSON core module
test.json: json.test.exe

json.test.exe: test/json.test.o core/polyfill.a extra/json.a
	g++ test/json.test.cpp core/polyfill.a extra/json.a -o json.test.exe $(CFLAGS)

test/json.test.o: test/json.test.cpp
	g++ -c test/json.test.cpp -o test/json.test.o $(CFLAGS)

# Test Storage extra module
test.storage: storage.test.exe

storage.test.exe: test/storage.test.o extra/storage.a core/encoding.a
	g++ test/storage.test.cpp extra/storage.a core/encoding.a -o storage.test.exe $(CFLAGS)

test/storage.test.o: test/storage.test.cpp
	g++ -c test/storage.test.cpp -o test/storage.test.o $(CFLAGS)
