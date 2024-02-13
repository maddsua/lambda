
LIB_CORE_WEBSOCKET		=	core/websocket.a
LIB_CORE_WEBSOCKET_DEPS	=	core/websocket/websocket.o core/websocket/transport.o

# websocket protocol
$(LIB_CORE_WEBSOCKET): $(LIB_CORE_WEBSOCKET_DEPS)
	ar rvs $(LIB_CORE_WEBSOCKET) $(LIB_CORE_WEBSOCKET_DEPS)

core/websocket/websocket.o: core/websocket/websocket.cpp
	g++ -c $(CFLAGS) core/websocket/websocket.cpp -o core/websocket/websocket.o

core/websocket/transport.o: core/websocket/transport.cpp
	g++ -c $(CFLAGS) core/websocket/transport.cpp -o core/websocket/transport.o
