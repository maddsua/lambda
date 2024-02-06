
LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_DEPS	=	core/server/instance.o core/server/connection.o core/server/handlers/serverless.o core/server/handlers/connection.o core/server/pages/errorpage.o

# server stuff
$(LIB_CORE_SERVER): $(LIB_CORE_SERVER_DEPS)
	ar rvs $(LIB_CORE_SERVER) $(LIB_CORE_SERVER_DEPS)

core/server/instance.o: core/server/instance.cpp
	g++ -c $(CFLAGS) core/server/instance.cpp -o core/server/instance.o

core/server/connection.o: core/server/connection.cpp
	g++ -c $(CFLAGS) core/server/connection.cpp -o core/server/connection.o

core/server/handlers/serverless.o: core/server/handlers/serverless.cpp
	g++ -c $(CFLAGS) core/server/handlers/serverless.cpp -o core/server/handlers/serverless.o

core/server/handlers/connection.o: core/server/handlers/connection.cpp
	g++ -c $(CFLAGS) core/server/handlers/connection.cpp -o core/server/handlers/connection.o

core/server/pages/errorpage.o: core/server/pages/errorpage.cpp
	g++ -c $(CFLAGS) core/server/pages/errorpage.cpp -o core/server/pages/errorpage.o
