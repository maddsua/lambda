
LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_DEPS	=	core/server/instance.o core/server/http/connection.o core/server/http/transport.o core/server/http/upgrade.o core/server/console.o core/server/websocket/context.o core/server/websocket/transport.o

# server stuff
$(LIB_CORE_SERVER): $(LIB_CORE_SERVER_DEPS)
	ar rvs $(LIB_CORE_SERVER) $(LIB_CORE_SERVER_DEPS)

core/server/instance.o: core/server/instance.cpp
	g++ -c $(CFLAGS) core/server/instance.cpp -o core/server/instance.o

core/server/http/connection.o: core/server/http/connection.cpp
	g++ -c $(CFLAGS) core/server/http/connection.cpp -o core/server/http/connection.o

core/server/http/transport.o: core/server/http/transport.cpp
	g++ -c $(CFLAGS) core/server/http/transport.cpp -o core/server/http/transport.o

core/server/http/upgrade.o: core/server/http/upgrade.cpp
	g++ -c $(CFLAGS) core/server/http/upgrade.cpp -o core/server/http/upgrade.o

core/server/console.o: core/server/console.cpp
	g++ -c $(CFLAGS) core/server/console.cpp -o core/server/console.o

core/server/websocket/context.o: core/server/websocket/context.cpp
	g++ -c $(CFLAGS) core/server/websocket/context.cpp -o core/server/websocket/context.o

core/server/websocket/transport.o: core/server/websocket/transport.cpp
	g++ -c $(CFLAGS) core/server/websocket/transport.cpp -o core/server/websocket/transport.o
