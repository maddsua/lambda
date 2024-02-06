
LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_DEPS	=	core/server/instance.o core/server/http/connection.o core/server/http/upgrade.o core/server/handlers/serverless.o core/server/handlers/connection.o core/server/pages/errorpage.o

# server stuff
$(LIB_CORE_SERVER): $(LIB_CORE_SERVER_DEPS)
	ar rvs $(LIB_CORE_SERVER) $(LIB_CORE_SERVER_DEPS)

core/server/instance.o: core/server/instance.cpp
	g++ -c $(CFLAGS) core/server/instance.cpp -o core/server/instance.o

core/server/http/connection.o: core/server/http/connection.cpp
	g++ -c $(CFLAGS) core/server/http/connection.cpp -o core/server/http/connection.o

core/server/http/upgrade.o: core/server/http/upgrade.cpp
	g++ -c $(CFLAGS) core/server/http/upgrade.cpp -o core/server/http/upgrade.o

core/server/handlers/serverless.o: core/server/handlers/serverless.cpp
	g++ -c $(CFLAGS) core/server/handlers/serverless.cpp -o core/server/handlers/serverless.o

core/server/handlers/connection.o: core/server/handlers/connection.cpp
	g++ -c $(CFLAGS) core/server/handlers/connection.cpp -o core/server/handlers/connection.o

core/server/pages/errorpage.o: core/server/pages/errorpage.cpp
	g++ -c $(CFLAGS) core/server/pages/errorpage.cpp -o core/server/pages/errorpage.o
