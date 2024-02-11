
# Test network/TCP core module
test.tcp: tcp.test$(EXEEXT)

tcp.test$(EXEEXT): tests/tcp/tcp.test.o $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR)
	g++ $(CFLAGS) tests/tcp/tcp.test.cpp $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR) $(LINK_SYSTEM_LIBS) -o tcp.test$(EXEEXT)

tests/tcp/tcp.test.o: tests/tcp/tcp.test.cpp
	g++ -c $(CFLAGS) tests/tcp/tcp.test.cpp -o tests/tcp/tcp.test.o
