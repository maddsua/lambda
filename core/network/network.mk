
LIB_CORE_NETWORK		=	core/network.a
LIB_CORE_NETWORK_DEPS	=	core/network/tcp/connection.o core/network/tcp/listen.o

# network stuff
$(LIB_CORE_NETWORK): $(LIB_CORE_NETWORK_DEPS)
	ar rvs $(LIB_CORE_NETWORK) $(LIB_CORE_NETWORK_DEPS)

core/network/tcp/connection.o: core/network/tcp/connection.cpp
	g++ -c $(CFLAGS) core/network/tcp/connection.cpp -o core/network/tcp/connection.o

core/network/tcp/listen.o: core/network/tcp/listener.cpp
	g++ -c $(CFLAGS) core/network/tcp/listener.cpp -o core/network/tcp/listen.o
