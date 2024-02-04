
LIB_CORE_HANDLERS			=	core/handlers.a
LIB_CORE_HANDLERS_DEPS		=	core/handlers/serverless.o core/handlers/stream.o

# handlers stuff
$(LIB_CORE_HANDLERS): $(LIB_CORE_HANDLERS_DEPS)
	ar rvs $(LIB_CORE_HANDLERS) $(LIB_CORE_HANDLERS_DEPS)

core/handlers/serverless.o: core/handlers/serverless.cpp
	g++ -c $(CFLAGS) core/handlers/serverless.cpp -o core/handlers/serverless.o

core/handlers/stream.o: core/handlers/stream.cpp
	g++ -c $(CFLAGS) core/handlers/stream.cpp -o core/handlers/stream.o
