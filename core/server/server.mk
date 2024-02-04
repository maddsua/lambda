
LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_PAGES	=	core/server/pages/resources/servicepage.res
LIB_CORE_SERVER_DEPS	=	core/server/instance.o core/server/http/connection.o core/server/http/transport.o core/server/http/upgrade.o core/server/console.o core/server/handlers/serverless.o core/server/handlers/connection.o core/server/websocket/context.o core/server/websocket/transport.o core/server/pages/templates.o core/server/pages/renderer.o core/server/pages/errorpage.o $(LIB_CORE_SERVER_PAGES)

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

core/server/handlers/serverless.o: core/server/handlers/serverless.cpp
	g++ -c $(CFLAGS) core/server/handlers/serverless.cpp -o core/server/handlers/serverless.o

core/server/handlers/connection.o: core/server/handlers/connection.cpp
	g++ -c $(CFLAGS) core/server/handlers/connection.cpp -o core/server/handlers/connection.o

core/server/console.o: core/server/console.cpp
	g++ -c $(CFLAGS) core/server/console.cpp -o core/server/console.o

core/server/websocket/context.o: core/server/websocket/context.cpp
	g++ -c $(CFLAGS) core/server/websocket/context.cpp -o core/server/websocket/context.o

core/server/websocket/transport.o: core/server/websocket/transport.cpp
	g++ -c $(CFLAGS) core/server/websocket/transport.cpp -o core/server/websocket/transport.o

core/server/pages/templates.o: core/server/pages/templates.cpp
	g++ -c $(CFLAGS) core/server/pages/templates.cpp -o core/server/pages/templates.o

core/server/pages/renderer.o: core/server/pages/renderer.cpp
	g++ -c $(CFLAGS) core/server/pages/renderer.cpp -o core/server/pages/renderer.o

core/server/pages/errorpage.o: core/server/pages/errorpage.cpp
	g++ -c $(CFLAGS) core/server/pages/errorpage.cpp -o core/server/pages/errorpage.o

core/server/pages/resources/servicepage.res: core/server/pages/resources/servicepage.html
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 core/server/pages/resources/servicepage.html core/server/pages/resources/servicepage.res
