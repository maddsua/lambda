
LIB_CORE_WEBSOCKET		=	core/websocket.a
LIB_CORE_WEBSOCKET_DEPS	=	core/websocket/message.o core/websocket/context.o core/websocket/transport.o

# websocket protocol
$(LIB_CORE_WEBSOCKET): $(LIB_CORE_WEBSOCKET_DEPS)
	ar rvs $(LIB_CORE_WEBSOCKET) $(LIB_CORE_WEBSOCKET_DEPS)

core/websocket/message.o: core/websocket/message.cpp
	g++ -c $(CFLAGS) core/websocket/message.cpp -o core/websocket/message.o

core/websocket/context.o: core/websocket/context.cpp
	g++ -c $(CFLAGS) core/websocket/context.cpp -o core/websocket/context.o

core/websocket/transport.o: core/websocket/transport.cpp
	g++ -c $(CFLAGS) core/websocket/transport.cpp -o core/websocket/transport.o
