
# Test network/TCP core module
test.tcp: tcp.test$(EXEEXT)

tcp.test$(EXEEXT): tests/tcp/main.o $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR)
	g++ $(CFLAGS) tests/tcp/main.cpp $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR) $(LINK_SYSTEM_LIBS) -o tcp.test$(EXEEXT)

tests/tcp/main.o: tests/tcp/main.cpp
	g++ -c $(CFLAGS) tests/tcp/main.cpp -o tests/tcp/main.o
