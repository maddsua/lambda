
# Test network/TCP core module
test.tcp: tcp.test$(EXEEXT)

tcp.test$(EXEEXT): test/tcp/main.o $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR)
	g++ $(CFLAGS) test/tcp/main.cpp $(LIB_CORE_NETWORK) $(LIB_CORE_ERROR) $(LINK_SYSTEM_LIBS) -o tcp.test$(EXEEXT)

test/tcp/main.o: test/tcp/main.cpp
	g++ -c $(CFLAGS) test/tcp/main.cpp -o test/tcp/main.o
