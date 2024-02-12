
LIB_CORE_SERVER			=	core/server.a
LIB_CORE_SERVER_DEPS	=	core/server/instance.o core/server/handler.o core/server/errorpage.o

# server stuff
$(LIB_CORE_SERVER): $(LIB_CORE_SERVER_DEPS)
	ar rvs $(LIB_CORE_SERVER) $(LIB_CORE_SERVER_DEPS)

core/server/instance.o: core/server/instance.cpp
	g++ -c $(CFLAGS) core/server/instance.cpp -o core/server/instance.o

core/server/handler.o: core/server/handler.cpp
	g++ -c $(CFLAGS) core/server/handlers.cpp -o core/server/handlers.o

core/server/errorpage.o: core/server/errorpage.cpp
	g++ -c $(CFLAGS) core/server/errorpage.cpp -o core/server/errorpage.o
